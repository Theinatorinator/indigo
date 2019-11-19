// LX200 simulator for Arduino
//
// Copyright (c) 2018 CloudMakers, s. r. o.
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef ARDUINO_SAM_DUE
#define Serial SerialUSB
#endif

//#define LCD
#define OLED

#include <stdarg.h>

#ifdef LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#endif

#ifdef OLED
#include "heltec.h"
#endif

#define DEC_PER_SEC 1440
#define RA_PER_SEC 96

bool is_meade = false;
bool is_10micron = false;
bool is_gemini = false;
bool is_avalon = false;
bool is_onestep = false;
bool is_rainbow = true;

int date_day = 1;
int date_month = 1;
int date_year = 2018;

int time_hour = 22;
int time_minute = 15;
int time_second = 30;
int time_offset = 2;
int time_dst = 1;

char latitude[16] = "+48*08";
char longitude[16] = "+17*06";

bool high_precision = true;

long ra = 0;
long dec = 90L * 360000L;

long target_ra = 0;
long target_dec = 90L * 360000L;

int ra_slew = 0;
int dec_slew = 0;

char tracking_rate = 'Q';
char slew_rate = 'M';

bool is_slewing = false;
bool is_tracking = true;
bool is_parked = false;

int focuser_position = 0;
int focuser_move = 0;
int focuser_speed = 1;

void update_state() {
  static long unsigned last_millis = 0;
  static unsigned long time_lapse = 0;
  long unsigned current_millis = millis();
  long unsigned lapse = current_millis - last_millis;
  time_lapse += current_millis - last_millis;
  last_millis = current_millis;
  if (is_slewing) {
    long diff = target_ra - ra;
    if (abs(diff) < RA_PER_SEC * lapse) {
      ra = target_ra;
    } else {
      ra = ra + (diff > 0 ? RA_PER_SEC : -RA_PER_SEC) * lapse;
    }
    diff = target_dec - dec;
    if (abs(diff) < DEC_PER_SEC * lapse) {
      dec = target_dec;
    } else {
      dec = dec + (diff > 0 ? DEC_PER_SEC : -DEC_PER_SEC) * lapse;
    }
    if (ra == target_ra && dec == target_dec)
      is_slewing = false;
  } else if (!is_tracking) {
    ra = (ra + lapse) % (24L * 60L * 60L);
  }
  if (ra_slew || dec_slew) {
    long rate;
    if (slew_rate == 'G')
      rate = 10 * lapse;
    else if (slew_rate == 'C')
      rate = 50 * lapse;
    else if (slew_rate == 'M')
      rate = 500 * lapse;
    else
      rate = 1500 * lapse;
    ra += ra_slew * rate / 15;
    ra = (ra + 24L * 360000L) % (24L * 360000L);
    dec += dec_slew * rate;
    if (dec < -90L * 360000L)
      dec = -90L * 360000L; 
    if (dec > 90L * 360000L)
      dec = 90L * 360000L; 
  }
  focuser_position += focuser_move * lapse;
  int s = time_second + time_lapse / 1000;
  int m = time_minute + s / 60;
  int h = time_hour + m / 60;
  time_second = s % 60;
  time_minute = m % 60;
  time_hour = h % 24;
  time_lapse %= 1000;
#ifdef LCD
  char buffer[17];
  sprintf(buffer, "%02d%02d%02d %02d%02d%02d %c%c", ra / 360000L, (ra / 6000L) % 60, (ra / 100L) % 60, dec / 360000L, (abs(dec) / 6000L) % 60, (abs(dec) / 100L) % 60, is_tracking ? 'T' : 't', is_slewing ? 'S' : 's');
  lcd.setCursor(0, 0);
  lcd.print(buffer);
  sprintf(buffer, "%02d%02d%02d %c %c%c %c %c%c", time_hour, time_minute, time_second, slew_rate, (ra_slew < 0 ? 'E' : ra_slew > 0 ? 'W' : '0'), (dec_slew < 0 ? 'S' : dec_slew > 0 ? 'N' : '0'), tracking_rate, high_precision ? 'H' : 'h', is_parked ? 'P' : 'p');
  //sprintf(buffer, "%02d%02d%02d %02d%02d%02d", target_ra / 360000L, (target_ra / 6000L) % 60, (target_ra / 100L) % 60, target_dec / 360000L, (target_dec / 6000L) % 60, (target_dec / 100L) % 60);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
#endif
#ifdef OLED
  char buffer[32];
  Heltec.display->clear();
  sprintf(buffer, "%02d:%02d:%02d %02d:%02d:%02d", ra / 360000L, (ra / 6000L) % 60, (ra / 100L) % 60, dec / 360000L, (abs(dec) / 6000L) % 60, (abs(dec) / 100L) % 60);
  Heltec.display->drawString(0, 0, buffer);
  sprintf(buffer, "%02d:%02d:%02d", time_hour, time_minute, time_second);
  Heltec.display->drawString(0, 20, buffer);
  sprintf(buffer, "%c%c%c%c %c %c%c %c", high_precision ? 'H' : 'h', is_parked ? 'P' : 'p', is_tracking ? 'T' : 't', is_slewing ? 'S' : 's', slew_rate, (ra_slew < 0 ? 'E' : ra_slew > 0 ? 'W' : '0'), (dec_slew < 0 ? 'S' : dec_slew > 0 ? 'N' : '0'), tracking_rate);
  Heltec.display->drawString(0, 40, buffer);
  Heltec.display->display();
#endif
}

void setup() {
#ifdef LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("LX200 simulator");
  lcd.setCursor(0, 1);
  lcd.print("Not connected");
#endif
#ifdef OLED
  Heltec.begin(true, false, true);
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "LX200 simulator");
  Heltec.display->drawString(0, 20, "Not connected");
  Heltec.display->display();
#endif
  if (is_rainbow) {
    Serial.begin(115200);
    strcpy(latitude, "-127*20'10.0");
    strcpy(longitude, "+36*20'60.0");
  } else {
    Serial.begin(9600);
    strcpy(latitude, "+48*08");
    strcpy(longitude, "+17*06");
  }
  Serial.setTimeout(1000);
  while (!Serial)
    ;
}

void loop() {
  update_state();
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 6) {
      Serial.print("P");
    } else if (ch == ':' || ch == '>') {
      char buffer[64];
      memset(buffer, 0, sizeof(buffer));
      Serial.readBytesUntil('#', buffer, sizeof(buffer));
      if (!strcmp(buffer, "GVP")) {
        if (is_meade)
          Serial.print("Autostar#");
        else if (is_10micron)
          Serial.print("10micron GM1000HPS#");
        else if (is_gemini)
          Serial.print("Losmandy Gemini#");
        else if (is_avalon)
					Serial.print("Avalon#");
				else if (is_onestep)
					Serial.print("On-Step#");
      } else if (!strcmp(buffer, "GVF")) {
				Serial.print("ETX Autostar|A|43Eg|Apr 03 2007@11:25:53#");
      } else if (!strcmp(buffer, "GVN")) {
				Serial.print("43Eg#");
      } else if (!strcmp(buffer, "GVD")) {
				Serial.print("Apr 03 2007#");
      } else if (!strcmp(buffer, "GVT")) {
        Serial.print("11:25:53#");
      } else if (!strncmp(buffer, "SC", 2)) {
				date_day = atoi(buffer + 5);
				date_month = atoi(buffer + 2);
				date_year = 2000 + atoi(buffer + 8);
				Serial.print("1Updating planetary data#                        #");
      } else if (!strcmp(buffer, "GC")) {
        sprintf(buffer, "%02d/%02d/%02d#", date_month, date_day, date_year % 100);
        Serial.print(buffer);
      } else if (!strncmp(buffer, "SG", 2)) {
				time_offset = atoi(buffer + 2);
				Serial.print("1");
      } else if (!strcmp(buffer, "GG")) {
        sprintf(buffer, "%+03d#", time_offset);
        Serial.print(buffer);
      } else if (!strncmp(buffer, "SH", 2)) {
        time_dst = atoi(buffer + 2);
      } else if (!strcmp(buffer, "GH")) {
        if (is_rainbow) {
          if (is_parked)
            Serial.print(":GHO#");
          else
            Serial.print(":GH0#");          
        } else {
          sprintf(buffer, "%d#", time_dst);
          Serial.print(buffer);
        }
      } else if (!strncmp(buffer, "SL", 2)) {
				time_hour = atoi(buffer + 2);
				time_minute = atoi(buffer + 5);
				time_second = atoi(buffer + 8);
				Serial.print("1");
      } else if (!strcmp(buffer, "GL")) {
        if (is_rainbow)
          sprintf(buffer, ":%02d:%02d:%02d#", time_hour, time_minute, time_second);
        else
        Serial.print(buffer);
          sprintf(buffer, "%02d:%02d:%02d#", time_hour, time_minute, time_second);
      } else if (!strncmp(buffer, "Sg", 2)) {
        strcpy(longitude, buffer + 2);
        if (!is_rainbow)
          Serial.print("1");
      } else if (!strcmp(buffer, "Gg")) {
        if (is_rainbow) {
          Serial.print(":Gg");
          Serial.print(longitude);
          Serial.print("#");
        } else {
          Serial.print(longitude);
          Serial.print("#");
        }
      } else if (!strncmp(buffer, "St", 2)) {
        strcpy(latitude, buffer + 2);
        if (!is_rainbow)
          Serial.print("1");
      } else if (!strcmp(buffer, "Gt")) {
        if (is_rainbow) {
          Serial.print(":Gt");
          Serial.print(latitude);
          Serial.print("#");
        } else {
          Serial.print(latitude);
          Serial.print("#");
        }
      } else if (!strcmp(buffer, "U0")) {
				high_precision = false;
      } else if (!strcmp(buffer, "U1")) {
				high_precision = true;
      } else if (!strcmp(buffer, "U")) {
        high_precision = !high_precision;
      } else if (!strcmp(buffer, "P")) {
        if (high_precision) {
          Serial.print("LOW  PRECISION");
          if (is_meade)
            high_precision = false;
        } else {
          Serial.print("HIGH PRECISION");
          if (is_meade)
            high_precision = true;
        }
      } else if (!strncmp(buffer, "Sr", 2)) {
        if  (buffer[7] == '.') {
          target_ra = atol(buffer + 2) * 360000L + atof(buffer + 5) * 60000L;
        } else {
          target_ra = atol(buffer + 2) * 360000L + atol(buffer + 5) * 6000L + atol(buffer + 8) * 100L;
        }
        Serial.print("1");
      } else if (!strcmp(buffer, "Gr")) {
        if (high_precision) {
          sprintf(buffer, "%02d:%02d:%02d#", target_ra / 360000L, (target_ra / 6000L) % 60, (target_ra / 100L) % 60);
        } else {
          sprintf(buffer, "%02d:%04.1f#", target_ra / 360000L, ((target_ra / 600L) % 600) / 10.0);
        }
        Serial.print(buffer);
      } else if (!strcmp(buffer, "GR")) {
        if (is_rainbow) {
          sprintf(buffer, ":%02d:%02d:%02d.0#", ra / 360000L, (ra / 6000L) % 60, (ra / 100L) % 60);          
        } else if (high_precision) {
          sprintf(buffer, "%02d:%02d:%02d#", ra / 360000L, (ra / 6000L) % 60, (ra / 100L) % 60);
        } else {
          sprintf(buffer, "%02d:%04.1f#", ra / 360000L, ((ra / 600L) % 600) / 10.0);
        }
        Serial.print(buffer);
      } else if (!strncmp(buffer, "Sd", 2)) {
        if  (buffer[8] == 0) {
          target_dec = atol(buffer + 2) * 360000L;
          if (target_dec < 0)
            target_dec -= atol(buffer + 6) * 6000L;
          else
            target_dec += atol(buffer + 6) * 6000L;
        } else {
          target_dec = atol(buffer + 2) * 360000L;
          if (target_dec < 0)
            target_dec -= atol(buffer + 6) * 6000L + atol(buffer + 9) * 100L;
          else
            target_dec += atol(buffer + 6) * 6000L + atol(buffer + 9) * 100L;
        }
        Serial.print("1");
      } else if (!strcmp(buffer, "Gd")) {
        if (high_precision) {
          sprintf(buffer, "%+03d*%02d'%02d#", target_dec / 360000L, (abs(target_dec) / 6000L) % 60, (abs(target_dec) / 100L) % 60);
        } else {
          sprintf(buffer, "%+03d*%02d#", target_dec / 360000L, (abs(target_dec) / 600L) % 60);
        }
        Serial.print(buffer);
      } else if (!strcmp(buffer, "GD")) {
        if (is_rainbow) {
          sprintf(buffer, ":%+03d*%02d'%02d.0#", dec / 360000L, (abs(dec) / 6000L) % 60, (abs(dec) / 100L) % 60);
        } else if (high_precision) {
          sprintf(buffer, "%+03d*%02d'%02d#", dec / 360000L, (abs(dec) / 6000L) % 60, (abs(dec) / 100L) % 60);
        } else {
          sprintf(buffer, "%+03d*%02d#", dec / 360000L, (abs(dec) / 600L) % 60);
        }
        Serial.print(buffer);
      } else if (!strcmp(buffer, "CM")) {
        ra = target_ra;
        dec = target_dec;
        Serial.print("M31 EX GAL MAG 3.5 SZ178.0'#");
      } else if (!strcmp(buffer, "MS")) {
        is_slewing = true;
        if (is_rainbow) {
          Serial.print(":MM0#");
        } else {
          Serial.print("0");
        }
      } else if (!strcmp(buffer, "D")) {
        if (is_slewing)
          Serial.print("*#");
        else
          Serial.print("#");
      } else if (!strcmp(buffer, "Q")) {
        is_slewing = false;
        ra_slew = 0;
        dec_slew = 0;
      } else if (!strcmp(buffer, "AP") || !strncmp(buffer, "190:192", 7) || !strcmp(buffer, "X122")) {
        is_tracking = true;
      } else if (!strcmp(buffer, "AL") || !strncmp(buffer, "190:191", 7) || !strcmp(buffer, "X120")) {
        is_tracking = false;
      } else if (!strcmp(buffer, "GW")) {
        if (is_tracking)
          Serial.print("PT1#");
        else if (is_parked)
          Serial.print("PNP#");         
        else
          Serial.print("PN1#");         
      } else if (!strcmp(buffer, "Gstat")) {
        if (is_slewing && is_parked)
          Serial.print("2#");         
        else if (is_parked)
          Serial.print("5#");         
        else if (is_slewing)
          Serial.print("6#");         
        else if (is_tracking)
          Serial.print("0#");
        else
          Serial.print("7#");
      } else if (!strcmp(buffer, "Gv")) {
        if (is_slewing)
          Serial.print("S");
        else if (is_tracking)
          Serial.print("T");
        else
          Serial.print("N");
			} else if (!strcmp(buffer, "X34")) {
        if (is_slewing)
          Serial.print("m55#");
        else if (is_tracking)
          Serial.print("m11#");
        else
          Serial.print("m00#");
      } else if (!strcmp(buffer, "hP") || !strcmp(buffer, "hC") || !strcmp(buffer, "X362") || !strcmp(buffer, "Ch")) {
        target_ra = 0;
        target_dec = 90L * 360000L;
        is_tracking = false;
        is_slewing = true;
        is_parked = true;
        if (is_rainbow)
          Serial.print(":CHO#");
      } else if (!strcmp(buffer, "PO") || !strcmp(buffer, "hW") || !strcmp(buffer, "X370")) {
        is_tracking = true;
        is_slewing = false;
        is_parked = false;
      } else if (!strcmp(buffer, "TQ") || !strncmp(buffer, "130:131", 7)) {
        tracking_rate = 'Q';
      } else if (!strcmp(buffer, "TS") || !strcmp(buffer, "TSOLAR") || !strncmp(buffer, "130:134", 7)) {
        tracking_rate = 'S';
      } else if (!strcmp(buffer, "TL") || !strncmp(buffer, "130:135", 7)) {
        tracking_rate = 'L';
      } else if (!strcmp(buffer, "TM")) {
        tracking_rate = 'M';
      } else if (!strcmp(buffer, "RG")) {
        slew_rate = 'G';
      } else if (!strcmp(buffer, "RC")) {
        slew_rate = 'C';
      } else if (!strcmp(buffer, "RM")) {
        slew_rate = 'M';
      } else if (!strcmp(buffer, "RS")) {
        slew_rate = 'S';
      } else if (!strcmp(buffer, "Mn")) {
        dec_slew = 1;
      } else if (!strcmp(buffer, "Qn")) {
        dec_slew = 0;
      } else if (!strcmp(buffer, "Ms")) {
        dec_slew = -1;
      } else if (!strcmp(buffer, "Qs")) {
        dec_slew = 0;
      } else if (!strcmp(buffer, "Mw")) {
        ra_slew = 1;
      } else if (!strcmp(buffer, "Qw")) {
        ra_slew = 0;
      } else if (!strcmp(buffer, "Me")) {
        ra_slew = -1;
      } else if (!strcmp(buffer, "Qe")) {
        ra_slew = 0;
      } else if (!strcmp(buffer, "F+")) {
        focuser_move = focuser_speed;
      } else if (!strcmp(buffer, "F-")) {
        focuser_move = -focuser_speed;
      } else if (!strcmp(buffer, "FS")) {
        focuser_speed = 1;
      } else if (!strcmp(buffer, "FF")) {
        focuser_speed = 5;
      } else if (!strcmp(buffer, "FQ")) {
        focuser_move = 0;
      } else if (!strcmp(buffer, "FP")) {
        sprintf(buffer, "%d#", focuser_position);
        Serial.print(buffer);
      } else if (!strcmp(buffer, "AT")) {
        if (is_tracking)
          Serial.print(":AT1#");
        else
          Serial.print(":AT0#");
      } else if (!strncmp(buffer, "Ck", 2)) {
        ra = atof(buffer + 2) * 360000L;
        dec = atof(buffer + 9) * 360000L;
      }
    }
  }
}
