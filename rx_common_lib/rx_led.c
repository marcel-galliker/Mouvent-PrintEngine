// ****************************************************************************
//
//	led.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <string.h>
#include "rx_led.h"


static void led_set_property(char *led, char *property, char *value);

//--- led_off -------------------------------------
void led_off(char *led)
{
	led_set_property(led, "trigger", 	"none");
	led_set_property(led, "brightness", "0");
}

//--- led_on --------------------------------------
void led_on(char *led)
{
	led_set_property(led, "trigger", 	"none");
	led_set_property(led, "brightness", "1");
}


//--- led_blink -----------------------------------
void led_blink(char *led, int interval)
{
	char str[10];
	sprintf(str, "%d", interval);
	led_set_property(led, "trigger", 	"timer");
	led_set_property(led, "delay_on", 	str);
	led_set_property(led, "delay_off", 	str);
}

//--- set_led_property -----------------------------------------
static void led_set_property(char *led, char *property, char *value)
{
	FILE *fp;
	char dir[100];

	sprintf(dir, "/sys/class/leds/%s/%s", led, property);

	if ((fp = fopen(dir, "w")) == NULL) {
		printf("Failed to open the file %s\n", dir);
	}
	else {
		fwrite(value, 1, strlen(value)+1, fp);
		fclose(fp);
	}
}
