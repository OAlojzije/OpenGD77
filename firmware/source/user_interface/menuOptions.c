/*
 * Copyright (C)2019 Roger Clark. VK3KYY / G4KYF
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <user_interface/menuSystem.h>
#include <user_interface/uiLocalisation.h>
#include "fw_settings.h"
#include "fw_wdog.h"

static void updateScreen(void);
static void handleEvent(int buttons, int keys, int events);
static bool	doFactoryReset;
enum OPTIONS_MENU_LIST { OPTIONS_MENU_TIMEOUT_BEEP=0,OPTIONS_MENU_FACTORY_RESET,OPTIONS_MENU_USE_CALIBRATION,
							OPTIONS_MENU_TX_FREQ_LIMITS,OPTIONS_MENU_BEEP_VOLUME,OPTIONS_MIC_GAIN_DMR,
							OPTIONS_MENU_KEYPAD_TIMER_LONG, OPTIONS_MENU_KEYPAD_TIMER_REPEAT, OPTIONS_MENU_DMR_MONITOR_CAPTURE_TIMEOUT,
							OPTIONS_MENU_SCAN_DELAY,OPTIONS_MENU_SCAN_MODE,
							NUM_OPTIONS_MENU_ITEMS};


int menuOptions(int buttons, int keys, int events, bool isFirstRun)
{
	if (isFirstRun)
	{
		gMenusCurrentItemIndex=0;
		doFactoryReset=false;
		updateScreen();
	}
	else
	{
		if (events!=0 && keys!=0)
		{
			handleEvent(buttons, keys, events);
		}
	}
	return 0;
}

static void updateScreen(void)
{
	int mNum = 0;
	static const int bufferLen = 17;
	char buf[bufferLen];

	UC1701_clearBuf();
	menuDisplayTitle(currentLanguage->options);

	// Can only display 3 of the options at a time menu at -1, 0 and +1
	for(int i = -1; i <= 1; i++)
	{
		mNum = menuGetMenuOffset(NUM_OPTIONS_MENU_ITEMS, i);

		switch(mNum)
		{
			case OPTIONS_MENU_TIMEOUT_BEEP:
				if (nonVolatileSettings.txTimeoutBeepX5Secs != 0)
				{
					snprintf(buf, bufferLen, "%s:%d", currentLanguage->timeout_beep, nonVolatileSettings.txTimeoutBeepX5Secs * 5);
					buf[bufferLen - 1] = 0;
				}
				else
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->timeout_beep, currentLanguage->off);
					buf[bufferLen - 1] = 0;
				}
				break;
			case OPTIONS_MENU_FACTORY_RESET:
				if (doFactoryReset == true)
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->factory_reset, currentLanguage->yes);
					buf[bufferLen - 1] = 0;
				}
				else
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->factory_reset, currentLanguage->no);
					buf[bufferLen - 1] = 0;
				}
				break;
			case OPTIONS_MENU_USE_CALIBRATION:
				if (nonVolatileSettings.useCalibration)
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->calibration, currentLanguage->on);
					buf[bufferLen - 1] = 0;
				}
				else
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->calibration, currentLanguage->off);
					buf[bufferLen - 1] = 0;
				}
				break;
			case OPTIONS_MENU_TX_FREQ_LIMITS:// Tx Freq limits
				if (nonVolatileSettings.txFreqLimited)
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->band_limits, currentLanguage->on);
					buf[bufferLen - 1] = 0;
				}
				else
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->band_limits, currentLanguage->off);
					buf[bufferLen - 1] = 0;
				}
				break;
			case OPTIONS_MENU_BEEP_VOLUME:// Beep volume reduction
				snprintf(buf, bufferLen, "%s:%ddB", currentLanguage->beep_volume, (2 - nonVolatileSettings.beepVolumeDivider) * 3);
				buf[bufferLen - 1] = 0;
				soundBeepVolumeDivider = nonVolatileSettings.beepVolumeDivider;
				break;
			case OPTIONS_MIC_GAIN_DMR:// DMR Mic gain
				snprintf(buf, bufferLen, "%s:%ddB", currentLanguage->dmr_mic_gain, (nonVolatileSettings.micGainDMR - 11) * 3);
				buf[bufferLen - 1] = 0;
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_LONG:// Timer longpress
				snprintf(buf, bufferLen, "%s:%1d.%1ds", currentLanguage->key_long, nonVolatileSettings.keypadTimerLong / 10, nonVolatileSettings.keypadTimerLong % 10);
				buf[bufferLen - 1] = 0;
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_REPEAT:// Timer repeat
				snprintf(buf, bufferLen, "%s:%1d.%1ds", currentLanguage->key_repeat, nonVolatileSettings.keypadTimerRepeat/10, nonVolatileSettings.keypadTimerRepeat % 10);
				buf[bufferLen - 1] = 0;
				break;
			case OPTIONS_MENU_DMR_MONITOR_CAPTURE_TIMEOUT:// DMR filtr timeout repeat
				snprintf(buf, bufferLen, "%s:%ds", currentLanguage->dmr_filter_timeout, nonVolatileSettings.dmrCaptureTimeout);
				buf[bufferLen - 1] = 0;
				break;
			case OPTIONS_MENU_SCAN_DELAY:// Scan hold and pause time
				snprintf(buf, bufferLen, "%s:%ds", currentLanguage->scan_delay, nonVolatileSettings.scanDelay);
				buf[bufferLen - 1] = 0;
				break;
			case OPTIONS_MENU_SCAN_MODE:// scanning mode
				if (nonVolatileSettings.scanModePause)
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->scan_mode, currentLanguage->pause);
				}
				else
				{
					snprintf(buf, bufferLen, "%s:%s", currentLanguage->scan_mode, currentLanguage->hold);
				}
				buf[bufferLen - 1] = 0;
				break;
		}

		menuDisplayEntry(i, mNum, buf);
	}

	UC1701_render();
	displayLightTrigger();
}

static void handleEvent(int buttons, int keys, int events)
{
	if (KEYCHECK_PRESS(keys,KEY_DOWN) && gMenusEndIndex!=0)
	{
		MENU_INC(gMenusCurrentItemIndex, NUM_OPTIONS_MENU_ITEMS);
	}
	else if (KEYCHECK_PRESS(keys,KEY_UP))
	{
		MENU_DEC(gMenusCurrentItemIndex, NUM_OPTIONS_MENU_ITEMS);
	}
	else if (KEYCHECK_PRESS(keys,KEY_RIGHT))
	{
		switch(gMenusCurrentItemIndex)
		{
			case OPTIONS_MENU_TIMEOUT_BEEP:
				if (nonVolatileSettings.txTimeoutBeepX5Secs < 4)
				{
					nonVolatileSettings.txTimeoutBeepX5Secs++;
				}
				break;
			case OPTIONS_MENU_FACTORY_RESET:
				doFactoryReset = true;
				break;
			case OPTIONS_MENU_USE_CALIBRATION:
				nonVolatileSettings.useCalibration=true;
				break;
			case OPTIONS_MENU_TX_FREQ_LIMITS:
				nonVolatileSettings.txFreqLimited=true;
				break;
			case OPTIONS_MENU_BEEP_VOLUME:
				if (nonVolatileSettings.beepVolumeDivider>0)
				{
					nonVolatileSettings.beepVolumeDivider--;
				}
				break;
			case OPTIONS_MIC_GAIN_DMR:// DMR Mic gain
				if (nonVolatileSettings.micGainDMR<15 )
				{
					nonVolatileSettings.micGainDMR++;
					setMicGainDMR(nonVolatileSettings.micGainDMR);
				}
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_LONG:
				if (nonVolatileSettings.keypadTimerLong<90)
				{
					nonVolatileSettings.keypadTimerLong++;
				}
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_REPEAT:
				if (nonVolatileSettings.keypadTimerRepeat<90)
				{
					nonVolatileSettings.keypadTimerRepeat++;
				}
				break;
			case OPTIONS_MENU_DMR_MONITOR_CAPTURE_TIMEOUT:
				if (nonVolatileSettings.dmrCaptureTimeout<90)
				{
					nonVolatileSettings.dmrCaptureTimeout++;
				}
				break;
			case OPTIONS_MENU_SCAN_DELAY:
				if (nonVolatileSettings.scanDelay<30)
				{
					nonVolatileSettings.scanDelay++;
				}
				break;
			case OPTIONS_MENU_SCAN_MODE:
				nonVolatileSettings.scanModePause=true;
				break;
		}
	}
	else if (KEYCHECK_PRESS(keys,KEY_LEFT))
	{

		switch(gMenusCurrentItemIndex)
		{
			case OPTIONS_MENU_TIMEOUT_BEEP:
				if (nonVolatileSettings.txTimeoutBeepX5Secs>0)
				{
					nonVolatileSettings.txTimeoutBeepX5Secs--;
				}
				break;
			case OPTIONS_MENU_FACTORY_RESET:
				doFactoryReset = false;
				break;
			case OPTIONS_MENU_USE_CALIBRATION:
				nonVolatileSettings.useCalibration=false;
				break;
			case OPTIONS_MENU_TX_FREQ_LIMITS:
				nonVolatileSettings.txFreqLimited=false;
				break;
			case OPTIONS_MENU_BEEP_VOLUME:
				if (nonVolatileSettings.beepVolumeDivider<10)
				{
					nonVolatileSettings.beepVolumeDivider++;
				}
				break;
			case OPTIONS_MIC_GAIN_DMR:// DMR Mic gain
				if (nonVolatileSettings.micGainDMR>0)
				{
					nonVolatileSettings.micGainDMR--;
					setMicGainDMR(nonVolatileSettings.micGainDMR);
				}
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_LONG:
				if (nonVolatileSettings.keypadTimerLong>1)
				{
					nonVolatileSettings.keypadTimerLong--;
				}
				break;
			case OPTIONS_MENU_KEYPAD_TIMER_REPEAT:
				if (nonVolatileSettings.keypadTimerRepeat>0)
				{
					nonVolatileSettings.keypadTimerRepeat--;
				}
				break;
			case OPTIONS_MENU_DMR_MONITOR_CAPTURE_TIMEOUT:
				if (nonVolatileSettings.dmrCaptureTimeout>1)
				{
					nonVolatileSettings.dmrCaptureTimeout--;
				}
				break;
			case OPTIONS_MENU_SCAN_DELAY:
				if (nonVolatileSettings.scanDelay>1)
				{
					nonVolatileSettings.scanDelay--;
				}
				break;
			case OPTIONS_MENU_SCAN_MODE:
				nonVolatileSettings.scanModePause=false;
				break;
		}
	}
	else if (KEYCHECK_SHORTUP(keys,KEY_GREEN))
	{
		if (doFactoryReset==true)
		{
			settingsRestoreDefaultSettings();
			watchdogReboot();
		}
		menuSystemPopAllAndDisplayRootMenu();
		return;
	}
	else if (KEYCHECK_SHORTUP(keys,KEY_RED))
	{
		menuSystemPopPreviousMenu();
		return;
	}
	updateScreen();
}
