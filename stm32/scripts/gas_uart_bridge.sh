#!/bin/sh

I2C_BUS=1
SENSOR_ADDR=0x08
UART_DEV=/dev/ttySTM2

stty -F "$UART_DEV" 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts

# Start sensor warming
i2cset -y "$I2C_BUS" "$SENSOR_ADDR" 0xFE

echo "Gas sensor UART bridge started."
echo "I2C bus : $I2C_BUS"
echo "Sensor  : $SENSOR_ADDR"
echo "UART    : $UART_DEV"

SEQ=1

read_sensor()
{
    CMD="$1"

    DATA=$(i2ctransfer -y "$I2C_BUS" w1@"$SENSOR_ADDR" "$CMD" r4 2>/dev/null)

    if [ $? -ne 0 ] || [ -z "$DATA" ]; then
        echo "ERROR: I2C read failed for command $CMD" >&2
        return 1
    fi

    set -- $DATA

    # i2ctransfer returns bytes such as:
    # 0xe5 0x00 0x00 0x00
    #
    # Convert the complete hexadecimal value directly.
    VALUE=$(( $1 | ($2 << 8) | ($3 << 16) | ($4 << 24) ))

    echo "$VALUE"
}

while true
do
    NO2=$(read_sensor 0x01)
    ETHANOL=$(read_sensor 0x03)
    CO=$(read_sensor 0x07)

    if [ -n "$NO2" ] && [ -n "$ETHANOL" ] && [ -n "$CO" ]; then

        MESSAGE=$(printf "GAS,%06d,%d,%d,%d" \
            "$SEQ" "$NO2" "$ETHANOL" "$CO")

        printf '%s\r\n' "$MESSAGE" > "$UART_DEV"

        echo "STM32 TX: $MESSAGE"

        SEQ=$((SEQ + 1))
    else
        echo "ERROR: Invalid sensor reading"
    fi

    sleep 2
done