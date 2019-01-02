#!/bin/bash

# Written by Thomas Stibor <thomas@stibor.net>
VERSION="0.1.6"

# ANSI color codes.
COL_R='\033[0;31m'
COL_G='\033[0;32m'
COL__='\033[0m' # No color.

# Arguments
DRIVER_TEST=0
CAPTURE_TEST=""
FORMAT_TEST=""

#---------------- Helper functions -----------------#
__realpath() {
    path=`eval echo "$1"`
    folder=$(dirname "$path")
    echo $(cd "$folder"; pwd)/$(basename "$path");
}

__log_error() {
    printf "[`date "+%Y-%m-%d %H:%M:%S"`] ${COL_R}ERROR${COL__} $@\n"
}

__log_info() {
    printf "[`date "+%Y-%m-%d %H:%M:%S"`] ${COL_G}INFO${COL__} $@\n"
}

__bin_exists() {
    [[ ! -f ${1} ]] && { __log_error "cannot find binary '${1}'" ; exit 1; }
}

__usage() {
    echo -e "usage: ${0}\n" \
	   "\t-d, --driver-test\n" \
	   "\t-c, --capture-test <indigo_ccd_driver>\n" \
	   "\t-f, --format-test <indigo_ccd_driver>\n" \
	   "\tversion ${VERSION}"
    exit 1
}

INDIGO_TEST_PATH=$(dirname $(__realpath $0))
INDIGO_PATH="${INDIGO_TEST_PATH}/.."
INDIGO_DRIVERS_PATH="${INDIGO_PATH}/build/drivers"
INDIGO_SERVER="${INDIGO_PATH}/build/bin/indigo_server"
INDIGO_PROP_TOOL="${INDIGO_PATH}/build/bin/indigo_prop_tool"
INDIGO_SERVER_PID=0
LD_LIBRARY_PATH="${INDIGO_PATH}/indigo_drivers/ccd_iidc/externals/libdc1394/build/lib"

#---------------- INDIGO functions -----------------#
__start_indigo_server() {
    [[ `ps aux | grep "[i]ndigo_server"` ]] && { __log_error "indigo server is already running" ; exit 1; }
    # Reset configuration, because all drivers are marked for load after load driver test
    rm -f ~/.indigo/Server.config
    eval "${INDIGO_SERVER}" &>/dev/null &disown;
    INDIGO_SERVER_PID=`ps aux | grep "[i]ndigo_server" | awk '{print $2}'`
    __log_info "indigo_server with PID: ${INDIGO_SERVER_PID} started"
    sleep 3
}

__stop_indigo_server() {
    [[ ! `ps aux | grep "[i]ndigo_server"` ]] && { __log_error "indigo server is not running" ; exit 1; }

    killall -9 indigo_server
    if [[ $? -ne 0 ]]; then
	__log_error "cannot kill indigo_server"
	exit 1
    else
	__log_info "killed indigo_server"
    fi
    sleep 1
}

__set_verify_property() {

    [[ "$#" -ne 5 ]] && { __log_error "wrong number of arguments"; exit 1; }

    local SET_ARG=$1
    local GET_ARG=$2
    local GREP_ARG=$3
    local TIME_ARG_SET=$4
    local TIME_ARG_LIST=$5

    ${INDIGO_PROP_TOOL} -t "${TIME_ARG_SET}" "${SET_ARG}" > /dev/null
    ${INDIGO_PROP_TOOL} list -t "${TIME_ARG_LIST}" "${GET_ARG}" | grep -q "${GREP_ARG}"
    if [[ $? -ne 0 ]]; then
	__log_error "setting property '${SET_ARG}' failed"
	exit 1
    fi
    __log_info "setting property '${SET_ARG}' was successful"
}

__echo_driver_name() {

    local DRIVER_NAME=$(${INDIGO_PROP_TOOL} list \
			    | grep "INFO.DEVICE_NAME" \
			    | grep -v "Guider\|(wheel)\|(focuser)\|DSLR" \
			    | awk '{split($0, a, "."); print a[1]}')

    echo "${DRIVER_NAME}"
}

#-------------- INDIGO test functions --------------#
__test_load_drivers() {

    local TIME_WAIT_SET=1
    local TIME_WAIT_LIST=1

    __start_indigo_server

    __log_info "starting test_load_drivers"
    # Skip SBIG due to external driver (in form of DMG file) for MacOS is needed.
    for n in $(find ${INDIGO_DRIVERS_PATH} -type f \
		    -not -name "*.so" \
    	     	    -not -name "*.a" \
		    -not -name "*.dylib" \
		    -not -name "*indigo_ccd_sbig*" \
	      );
    do
	DRIVER=`basename ${n}`
	${INDIGO_PROP_TOOL} -t "${TIME_WAIT_SET}" "Server.LOAD.DRIVER=${DRIVER}" > /dev/null
	if [[ `${INDIGO_PROP_TOOL} list -t "${TIME_WAIT_LIST}" "Server.DRIVERS" | grep -q "${DRIVER} = ON"` ]]; then
	    __log_error "failed to load driver '${DRIVER}'"
	    exit 1
	fi
	__log_info "successfully loaded driver '${DRIVER}'"
    done

    __stop_indigo_server
}

__test_capture() {

    [[ "$#" -ne 1 ]] && { log_error "wrong number of arguments"; exit 1; }

    __start_indigo_server

    local TMPDIR="$(mktemp -q -d -t "$(basename "$0").XXXXXX" 2>/dev/null || mktemp -q -d)/"
    local DRIVER_NAME=""
    local TIME_WAIT_SET=1
    local TIME_WAIT_LIST=1

    # Some older DSLR having timing problem when connecting, thiis give them some time.
    [[ "${1}" == "indigo_ccd_gphoto2" ]] && { TIME_WAIT_LIST=4; }

    # Note, if longer exposure times are added, then make sure the
    # indigo_prop_tool time out is properly set. At current it is set to 1 second.
    declare -a EXP_TIMES=("0.01" "0.05" "0.1" "0.5" "1")

    # Load CCD driver.
    __set_verify_property "Server.LOAD.DRIVER=${1}" \
    			  "Server.DRIVERS" \
			  "${1} = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    TIME_WAIT_LIST=1
    DRIVER_NAME=$(__echo_driver_name)

    # Verify driver is loaded.
    __set_verify_property "${DRIVER_NAME}.CONNECTION.CONNECTED=ON" \
			  "${DRIVER_NAME}.CONNECTION" \
    			  "CONNECTED = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    # Set image format.
    __set_verify_property "${DRIVER_NAME}.CCD_IMAGE_FORMAT.RAW=ON" \
			  "${DRIVER_NAME}.CCD_IMAGE_FORMAT" \
			  "RAW = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    # Set local mode.
    __set_verify_property "${DRIVER_NAME}.CCD_UPLOAD_MODE.LOCAL=ON" \
			  "${DRIVER_NAME}.CCD_UPLOAD_MODE" \
    			  "LOCAL = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    # Set directory.
    __set_verify_property "${DRIVER_NAME}.CCD_LOCAL_MODE.DIR=${TMPDIR}" \
			  "${DRIVER_NAME}.CCD_LOCAL_MODE" \
    			  "DIR = \"${TMPDIR}\"" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    local cnt=0
    local fn=""
    local WAIT_TIME=0
    for e in "${EXP_TIMES[@]}"
    do
	if [[ "${1}" == "indigo_ccd_gphoto2" ]]; then
	    WAIT_TIME=$(echo ${e} | awk '{print int($1+5)}')
	else
	    WAIT_TIME=$(echo ${e} | awk '{print int($1+2)}')
	fi
	__set_verify_property "${DRIVER_NAME}.CCD_EXPOSURE.EXPOSURE=${e}" \
			      "${DRIVER_NAME}.CCD_EXPOSURE" \
    			      "EXPOSURE = 0" \
			      "${WAIT_TIME}" \
			      "1"
	(( cnt++ ))

	# Check RAW file exists.
	if [[ "${1}" == "indigo_ccd_gphoto2" ]]; then
	    fn="${TMPDIR}IMAGE_$(printf "%03d" ${cnt}).CR2"
	else
	    fn="${TMPDIR}IMAGE_$(printf "%03d" ${cnt}).raw"
	fi
	[[ ! -f  ${fn} ]] && { __log_error "exposure '${e} secs' failed, RAW file '${fn}' does not exist"; exit 1; }
	__log_info "RAW file '${fn}' of size $(wc -c ${fn} | awk '{print $1}') bytes successfully created"
    done

    __stop_indigo_server
}

__test_format() {

    [[ "$#" -ne 1 ]] && { log_error "wrong number of arguments"; exit 1; }

    __start_indigo_server

    local DRIVER_NAME=""
    local TMPDIR="$(mktemp -q -d -t "$(basename "$0").XXXXXX" 2>/dev/null || mktemp -q -d)/"
    declare -a IMG_FORMAT=("FITS" "XISF" "JPEG" "RAW")
    local EXP_TIME=3 # Note, set exposure to 3 seconds to get it work also with buggy ASI120/130mm USB 2.0 cameras.
    local TIME_WAIT_EXP=$(echo ${EXP_TIME} | awk '{print int($1+4)}')
    local TIME_WAIT_LIST=1
    local TIME_WAIT_SET=1

    if [[ "${1}" == "indigo_ccd_simulator" ]]; then
	EXP_TIME=0.5
	TIME_WAIT_EXP=$(echo ${EXP_TIME} | awk '{print int($1+2)}')
    elif [[ "${1}" == "indigo_ccd_gphoto2" ]]; then
	TIME_WAIT_EXP=$(echo ${EXP_TIME} | awk '{print int($1+10)}')
	TIME_WAIT_LIST=4
    fi

    # Load CCD driver.
    __set_verify_property "Server.LOAD.DRIVER=${1}" \
			  "Server.DRIVERS" \
			  "${1} = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    TIME_WAIT_LIST=1
    DRIVER_NAME=$(__echo_driver_name)

    # Verify driver is loaded.
    __set_verify_property "${DRIVER_NAME}.CONNECTION.CONNECTED=ON" \
			  "${DRIVER_NAME}.CONNECTION" \
			  "CONNECTED = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    # Set local mode.
    __set_verify_property "${DRIVER_NAME}.CCD_UPLOAD_MODE.LOCAL=ON" \
			  "${DRIVER_NAME}.CCD_UPLOAD_MODE" \
			  "LOCAL = ON" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    # Set directory.
    __set_verify_property "${DRIVER_NAME}.CCD_LOCAL_MODE.DIR=${TMPDIR}" \
			  "${DRIVER_NAME}.CCD_LOCAL_MODE" \
			  "DIR = \"${TMPDIR}\"" \
			  "${TIME_WAIT_SET}" \
			  "${TIME_WAIT_LIST}"

    local fn=""
    for f in "${IMG_FORMAT[@]}"
    do
	__set_verify_property "${DRIVER_NAME}.CCD_IMAGE_FORMAT.${f}=ON" \
			      "${DRIVER_NAME}.CCD_IMAGE_FORMAT" \
			      "${f} = ON" \
			      "${TIME_WAIT_SET}" \
			      "${TIME_WAIT_LIST}"

	__set_verify_property "${DRIVER_NAME}.CCD_EXPOSURE.EXPOSURE=${EXP_TIME}" \
			      "${DRIVER_NAME}.CCD_EXPOSURE" \
			      "EXPOSURE = 0" \
			      "${TIME_WAIT_EXP}" \
			      "${TIME_WAIT_LIST}"

	fn="${TMPDIR}IMAGE_$(printf "%03d" 001).`echo "${f}" | awk '{print tolower($0)}'`"
	local file_size=$(wc -c ${fn} | awk '{print $1}')
	[[ ! -f  ${fn} ]] && { __log_error "exposure '0.5 secs' failed, ${f} file '${fn}' does not exist"; exit 1; }
	[[ ${file_size} -eq 0 ]] && { __log_error "file '${fn}' has size 0 bytes"; exit 1; }

	if [[ "${f}" == "FITS" ]]; then
	    $(file -b "${fn}" | grep -q "^FITS image data")
	    [[ $? -ne 0 ]] && { __log_error "file '${fn}' is not in format '${f}'"; exit 1; }
	elif [[ "${f}" == "XISF" ]]; then
	    $(head -c 8 "${fn}" | grep -q "^XISF0100")
	    [[ $? -ne 0 ]] && { __log_error "file '${fn}' is not in format '${f}'"; exit 1; }
	elif [[ "${f}" == "JPEG" ]]; then
	    $(file -b "${fn}" | grep -q "^JPEG image data")
	    [[ $? -ne 0 ]] && { __log_error "file '${fn}' is not in format '${f}'"; exit 1; }
	elif [[ "${f}" == "RAW" ]]; then
	    $(file -b "${fn}" | grep -q "^data")
	    [[ $? -ne 0 ]] && { __log_error "file '${fn}' is not in format '${f}'"; exit 1; }
	fi

	__log_info "${f} file '${fn}' of size ${file_size} bytes successfully created"
    done

    __stop_indigo_server
}

#------------ Check required files exist -----------#
__bin_exists ${INDIGO_SERVER}
__bin_exists ${INDIGO_PROP_TOOL}

# Parse arguments.
while [[ $# -gt 0 ]]
do
    arg="$1"
    case $arg in
	-d|--driver-test)
	    DRIVER_TEST=1
	    ;;
	-c|--capture-test)
	    CAPTURE_TEST="$2"
	    [[ -z "${CAPTURE_TEST}" ]] && { echo "<indigo_ccd_driver> argument is missing"; __usage; }
	    shift
	    ;;
	-f|--format-test)
	    FORMAT_TEST="$2"
	    [[ -z "${FORMAT_TEST}" ]] && { echo "<indigo_ccd_driver> argument is missing"; __usage; }
	    shift
	    ;;
	*)
	    echo "unknown argument ${2}"
	    __usage
	    ;;
    esac
    shift
done

# Check for missing arguments.
[[ ${DRIVER_TEST} -eq 0 ]] && [[ -z "${CAPTURE_TEST}" ]] && [[ -z "${FORMAT_TEST}" ]] &&
    { echo "missing which test to run"; __usage; }

#------------------- Tests INDIGO ------------------#
[[ ${DRIVER_TEST} -ne 0 ]] && __test_load_drivers
[[ ! -z "${CAPTURE_TEST}" ]] && __test_capture "${CAPTURE_TEST}"
[[ ! -z "${FORMAT_TEST}" ]] && __test_format "${FORMAT_TEST}"
exit 0
