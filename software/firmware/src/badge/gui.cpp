#include "gui.h"
#include "stm32f3xx_hal.h"

void GUI_ListItemData::setShouldScroll() {
	if (strlen(text) > 14) {
		resetScrollable();
	} else {
		Scrollable = 0;
	}
}

GUI_TickerData::GUI_TickerData(const char * txt, uint8_t X, uint8_t Y,
		uint8_t W, uint8_t H) :
		text(txt), x(X), y(Y), w(W), h(H), BorderSize(1), FontScalar(1), bg(
				RGBColor::BLACK), TextColor(RGBColor::WHITE), startTick(0) {
}

GUI::GUI(DisplayST7735 *display) :
		Display(display) {

}

bool GUI::init() {
	return true;
}

void GUI::drawTicker(GUI_TickerData *dt) {
	uint8_t maxlen = (dt->w - dt->BorderSize * 2)
			/ Display->getFont()->FontWidth, len = 0;
	int shift = 0;
	while (*(dt->text + len) != 0)
		len++;
	if (dt->startTick == 0) {
		dt->startTick = HAL_GetTick();
	}
	shift = ((HAL_GetTick() - dt->startTick) / GUI_TickerSpeed) - maxlen / 2; //start delay

	if (shift > len - maxlen) {
		if (shift > len - maxlen + GUI_TickerEndDelay) {
			shift = 0;
			dt->startTick = HAL_GetTick();
		} else
			shift = len - maxlen;
	}
	if (shift < 0) {
		shift = 0;
	}
	Display->drawString(dt->x, dt->y, dt->text + shift, dt->TextColor, dt->bg,
			dt->FontScalar, false);
}

const char *GUI_ListItemData::getScrollOffset() {
	uint16_t offSet = 0;
	if (Scrollable) {
		if (LastScrollTime == 0) {
			LastScrollTime = HAL_GetTick();
		}
		if (HAL_GetTick() - LastScrollTime > TimeBetweenScroll) {
			LastScrollTime = HAL_GetTick();
			LastScrollPosition++;
			uint32_t l = strlen(text);
			//char b[10];
			//sprintf(&b[0],"%d",l);
			if (LastScrollPosition >= l) {
				LastScrollPosition = 0;
			}
		}
		offSet = LastScrollPosition;
	}
	return text + offSet;
}

uint8_t GUI::drawList(GUI_ListData* gui_CurList) {
	if (gui_CurList == 0)
		return 0;
	//SSD1306_DrawFilledRectangle(gui_CurList->x, gui_CurList->y, gui_CurList->w, gui_CurList->h, 0);
	//SSD1306_DrawRectangle(gui_CurList->x, gui_CurList->y, gui_CurList->w, gui_CurList->h, 1);

	uint8_t ry = gui_CurList->y + 2;
	if (gui_CurList->header != 0) {
		Display->drawString(gui_CurList->x + 1, gui_CurList->y, gui_CurList->header, RGBColor::WHITE, RGBColor::BLACK,
				1, false);
		ry += GUI_DefFont.FontHeight;
	}

	uint8_t maxC = ((gui_CurList->h - 3) / Display->getFont()->FontHeight)
			- (gui_CurList->header != 0);

	uint16_t i;
	if (maxC >= gui_CurList->ItemsCount) {
		for (i = 0; i < gui_CurList->ItemsCount; i++) {
			if (i != gui_CurList->selectedItem) {
				Display->drawString(gui_CurList->x + 1, ry + i * Display->getFont()->FontHeight,
						gui_CurList->items[i].text, RGBColor::WHITE, RGBColor::BLACK, 1, false);
			} else {
				Display->drawString(gui_CurList->x + 1, ry + i * Display->getFont()->FontHeight,
						gui_CurList->items[i].getScrollOffset(), RGBColor::BLACK, RGBColor::WHITE, 1, false);
			}
		}
	} else {
		if (gui_CurList->ItemsCount - 1 - gui_CurList->selectedItem
				< maxC / 2) {
			for (i = gui_CurList->ItemsCount - maxC;
					i < gui_CurList->ItemsCount; i++) {
				if (i != gui_CurList->selectedItem) {
					Display->drawString(gui_CurList->x + 1,
							ry + (i - gui_CurList->ItemsCount + maxC) * Display->getFont()->FontHeight,
							gui_CurList->items[i].text, RGBColor::WHITE, RGBColor::BLACK, 1, false);
				} else {
					Display->drawString(gui_CurList->x + 1,
							ry + (i - gui_CurList->ItemsCount + maxC) * Display->getFont()->FontHeight,
							gui_CurList->items[i].getScrollOffset(), RGBColor::BLACK, RGBColor::WHITE, 1, false);
				}
			}
		} else if (gui_CurList->selectedItem < maxC / 2) {
			for (i = 0; i < maxC; i++) {
				if (i != gui_CurList->selectedItem)
					Display->drawString(gui_CurList->x + 1, ry + i * Display->getFont()->FontHeight,
							gui_CurList->items[i].text, RGBColor::WHITE, RGBColor::BLACK, 1, false);
				else
					Display->drawString(gui_CurList->x + 1, ry + i * Display->getFont()->FontHeight,
							gui_CurList->items[i].getScrollOffset(), RGBColor::BLACK, RGBColor::WHITE, 1, false);
			}
		} else {
			for (i = gui_CurList->selectedItem - maxC / 2;
					i < gui_CurList->selectedItem - maxC / 2 + maxC; i++) {
				if (i != gui_CurList->selectedItem) {
				Display->drawString(gui_CurList->x + 1,
						ry + (i - gui_CurList->selectedItem + maxC / 2) * Display->getFont()->FontHeight,
						gui_CurList->items[i].text, RGBColor::WHITE, RGBColor::BLACK, 1, false);
				} else {
					Display->drawString(gui_CurList->x + 1,
						ry + (i - gui_CurList->selectedItem + maxC / 2) * Display->getFont()->FontHeight,
						gui_CurList->items[i].getScrollOffset(), RGBColor::BLACK, RGBColor::WHITE, 1, false);
				}
			}
		}
	}
	uint8_t sli_h = gui_CurList->h / gui_CurList->ItemsCount;
	if (sli_h < 10)
		sli_h = 10;
	uint8_t yy = ((gui_CurList->h) * gui_CurList->selectedItem)
			/ gui_CurList->ItemsCount;
	Display->drawHorizontalLine(gui_CurList->x, ry - 2,
			gui_CurList->x + gui_CurList->w, RGBColor::WHITE);
//SSD1306_DrawLine(gui_CurList->x + gui_CurList->w - 1, gui_CurList->y + yy, gui_CurList->x + gui_CurList->w - 1,
//	gui_CurList->y + yy + sli_h, 1);
	return 0;
}
