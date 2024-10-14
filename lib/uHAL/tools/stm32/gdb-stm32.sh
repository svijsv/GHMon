#!/bin/sh
# NOTES:
#   https://openocd.org/doc/html/GDB-and-OpenOCD.html
#   https://johnnysswlab.com/gdb-a-quick-guide-to-make-your-debugging-easier/
#   https://ncrmnt.org/2019/04/14/note-to-self-how-to-flash-bin-files-via-raw-gdb/
#
if [ -z "${1}" ] || [ "X${1}" = "X-h" ] || [ "X${1}" = "X--help" ]; then
	cat >&2 <<-EOF
		Usage: ${0} <device_name> [openocd_cfg_file_name [...]]

		Environment variables:
			\$ELF_FILE:    Path to ELF file (./pio/build/<device>_debug/firmware.elf)
			\$LISTEN_PORT: Port to connect to for the remove debugging server (none; start openocd and use a pipe)
			     Tested with OpenOCD but ST-UTIL may work too
			\$GDB_LAYOUT:  Layout to use with GDB (src)
			\$GDB_EXE:     GDB executable (gdb-multiarch)
			\$OPENOCD_EXE: OpenOCD executable file name (openocd)
			     Only available if \$LISTEN_PORT is 'pipe'
			\$OPENOCD_LOG: Path to OpenOCD log file (./openocd.log)
			     Only available if \$LISTEN_PORT is 'pipe'
	EOF
	exit 1
fi
DEVICE="${1}"; shift

while [ ! -z "${1}" ]; do
	OPENOCD_CFG_FILES="${OPENOCD_CFG_FILES} --file ${1}"; shift
done

ELF_FILE=".pio/build/${DEVICE}_debug/firmware.elf"

OPENOCD_EXE="${OPENOCD_EXE:-openocd}"
OPENOCD_LOG="${OPENOCD_LOG:-openocd.log}"

# OpenOCD default port 3333
# st-util default port 4242
LISTEN_PORT="${LISTEN_PORT:-}"

GDB_LAYOUT="${GDB_LAYOUT:-src}"
if [ ! -z "${GDB_LAYOUT}" ]; then
	GDB_LAYOUT="layout ${GDB_LAYOUT}"
fi

GDB_EXE="${GDB_EXE:-gdb-multiarch}"


TMPDIR="${TMPDIR:-/tmp}"
GDB_CMD_FILE="$(mktemp --tmpdir gdb-stm32.cmd.XXXXXX)"

if [ -z "${OPENOCD_CFG_FILES}" ]; then
	if [ -e ./openocd.cfg ]; then
		OPENOCD_CFG_FILES="--file ./openocd.cfg"
	else
		case "${DEVICE}" in
			stm32f1* ) OPENOCD_CFG_FILES="--file target/stm32f1x.cfg" ;;
			stm32f2* ) OPENOCD_CFG_FILES="--file target/stm32f2x.cfg" ;;
			stm32f3* ) OPENOCD_CFG_FILES="--file target/stm32f3x.cfg" ;;
			stm32f4* ) OPENOCD_CFG_FILES="--file target/stm32f4x.cfg" ;;
			stm32f7* ) OPENOCD_CFG_FILES="--file target/stm32f7x.cfg" ;;
			* )
				echo "Unknown architecture, please manually specify the OpenOCD configuration file" >&2
				exit 1
				;;
		esac
	fi
fi
if [ -z "${LISTEN_PORT}" ] || [ "${LISTEN_PORT}" = "pipe" ]; then
	REMOTE_SRV="| ${OPENOCD_EXE} --file interface/stlink.cfg ${OPENOCD_CFG_FILES} --log_output \"${OPENOCD_LOG}\" -c \"gdb_port pipe\""
else
	REMOTE_SRV="localhost:${LISTEN_PORT}"
fi

cat >"${GDB_CMD_FILE}" <<EOF
target extended-remote ${REMOTE_SRV}
${GDB_LAYOUT}
tui focus cmd
#tui refresh
EOF

${GDB_EXE} --tui --command="${GDB_CMD_FILE}" "${ELF_FILE}"

rm -f "${GDB_CMD_FILE}"
