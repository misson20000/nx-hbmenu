#include "common.h"

#include "switchicon_questionmark_bin.h"
#include "folder_icon_bin.h"

void launchMenuEntryTask(menuEntry_s* arg)
{
    menuEntry_s* me = arg;
    if (me->type == ENTRY_TYPE_FOLDER)
        menuScan(me->path);
        //changeDirTask(me->path);
    else
        launchMenuEntry(me);
}

//Draws a RGB888 image.
static void drawImage(int x, int y, int width, int height, int dw, int dh, const uint8_t *image) {
    int tmpx, tmpy;
    int pos;

    for (tmpx=0; tmpx < dw; tmpx++) {
        for (tmpy=0; tmpy < dh; tmpy++) {
	        int sx = tmpx * width / dw;
	        int sy = tmpy * height / dh;
            pos = ((sy*width) + sx) * 3;
            DrawPixelRaw(x+tmpx, y+tmpy, MakeColor(image[pos+0], image[pos+1], image[pos+2], 255));
        }
    }
}

static void drawEntry(menuEntry_s* me, int n, int is_active, int start_x, int start_y, int image_size) {
    int x, y;
    //int start_y = 96 + 108 - 32;//*(n % 2);
    int end_y = start_y + image_size + 32;
    //int start_x = 64 + (256+16)*n;//(n / 2);
    int end_x = start_x + image_size;
    uint8_t *imageptr = NULL;
    char tmpstr[1024];

    color_t border_color0 = MakeColor(255, 255, 255, 255);
    color_t border_color1 = MakeColor(255, 255, 255, 255);

    if (is_active) {
        border_color0 = MakeColor(171, 224, 245, 255);
        border_color1 = MakeColor(189, 228, 242, 255);
    }

    //{
        for (x=(start_x-5); x<(end_x+5); x++) {
            if (!is_active) {
                if (x < start_x-3)
                    continue;
                else if (x >= end_x+3)
                    break;
            }

            //DrawPixelRaw(x, start_y  , border_color0);
            DrawPixelRaw(x, end_y    , border_color0);
            DrawPixelRaw(x, start_y-1, border_color0);
            DrawPixelRaw(x, end_y  +1, border_color0);
            DrawPixelRaw(x, start_y-2, border_color0);
            DrawPixelRaw(x, end_y  +2, border_color0);
            
            if (is_active) {
                DrawPixelRaw(x, start_y-3, border_color0);
                DrawPixelRaw(x, end_y  +3, border_color0);
                DrawPixelRaw(x, start_y-4, border_color0);
                DrawPixelRaw(x, end_y  +4, border_color0);
                DrawPixelRaw(x, start_y-5, border_color1);
                DrawPixelRaw(x, end_y  +5, border_color1);
            }
            else {
                DrawPixelRaw(x, start_y-3, border_color1);
                DrawPixelRaw(x, end_y  +3, border_color1);
            }
        }

        for (y=(start_y-5); y<(end_y+5); y++) {
            if (!is_active) {
                if (y < start_y-3)
                    continue;
                else if (y >= end_y+3)
                    break;
            }
            
            DrawPixelRaw(start_x  , y, border_color0);
            DrawPixelRaw(end_x    , y, border_color0);
            DrawPixelRaw(start_x-1, y, border_color0);
            DrawPixelRaw(end_x  +1, y, border_color0);
            DrawPixelRaw(start_x-2, y, border_color0);
            DrawPixelRaw(end_x  +2, y, border_color0);

            if (is_active) {
                DrawPixelRaw(start_x-3, y, border_color0);
                DrawPixelRaw(end_x  +3, y, border_color0);
                DrawPixelRaw(start_x-4, y, border_color0);
                DrawPixelRaw(end_x  +4, y, border_color0);
                DrawPixelRaw(start_x-5, y, border_color1);
            }
            else {
                DrawPixelRaw(start_x-3, y, border_color1);
                //DrawPixelRaw(end_x  +3, y, border_color1);
            }
        }
    //}

    for (x=start_x; x<end_x; x++) {
        for (y=start_y; y<end_y; y++) {
            DrawPixelRaw(x, y, MakeColor(255, 255, 255, 255));
        }
    }

    if (me->icon_gfx)
        imageptr = me->icon_gfx;
    else if (me->type == ENTRY_TYPE_FOLDER)
        imageptr = (uint8_t*)folder_icon_bin;
    else
        imageptr = (uint8_t*)switchicon_questionmark_bin;

    if (imageptr) drawImage(start_x, start_y+32, 256, 256, image_size, image_size, imageptr);

    DrawTextTruncate(tahoma12, start_x + 8, start_y + 8, MakeColor(64, 64, 64, 255), me->name, 256 - 32, "...");
}

void menuStartup() {
    const char *path;

    #ifdef SWITCH
    path = "sdmc:/switch";
    #else
    path = "switch";
    #endif

    menuScan(path);
}

color_t waveBlendAdd(color_t a, color_t b, float alpha) {
    return MakeColor(a.r+(b.r*alpha), a.g+b.g*alpha, a.b + b.b*alpha, 255);
}

const int ENABLE_WAVE_BLENDING = 1;
double timer;

void drawWave(float timer, color_t color, float height, float phase, float speed) {
    int x, y;
    double wave_top_y, alpha;
    color_t existing_color, new_color;
    
    height = 720 - height;

    for (x=0; x<1280; x++) {
        wave_top_y = approxSin(x*speed/1280.0+timer+phase) * 10.0 + height;

        for (y=wave_top_y; y<720; y++) {
            alpha = clamp(y-wave_top_y, 0.0, 1.0) * 0.3;
            existing_color = FetchPixelColor(x, y);
            new_color = ENABLE_WAVE_BLENDING ? waveBlendAdd(existing_color, color, alpha) : color;

            DrawPixelRaw(x, y, new_color);
        }
    }
}

static float state = 0.0; // zero is grid view, one is individual view
static float targetState = 0.0;

static void drawDetails(menuEntry_s* me) {

	uint8_t *imageptr = NULL;
	
	if (me->icon_gfx)
		imageptr = me->icon_gfx;
	else if (me->type == ENTRY_TYPE_FOLDER)
		imageptr = (uint8_t*)folder_icon_bin;
	else
		imageptr = (uint8_t*)switchicon_questionmark_bin;

	const int imageSize = 256;
	const int offScreen = 300;
	const int rightMargin = 250;
	
	const int imageX = 1280 + offScreen - ((offScreen + rightMargin + imageSize) * state);
	
	if (imageptr) drawImage(imageX, 128, 256, 256, imageSize, imageSize, imageptr);

	const int leftMargin = 250;
	
	const int textX = -offScreen + ((offScreen + leftMargin) * state);
	
	DrawText(tahoma24, textX, 128, MakeColor(255, 255, 255, 255), me->name);

	char tmpstr[1024];
	memset(tmpstr, 0, sizeof(tmpstr));
	snprintf(tmpstr, sizeof(tmpstr)-1, "Placeholder description text.\n\nAuthor: %s\nVersion: %s", me->author, me->version);
	DrawText(tahoma12, textX, 128 + 64, MakeColor(255, 255, 255, 255), tmpstr);
}

static float scrollIdx = 4.0;
static float targetScrollIdx = 4.0;
static float lastBump = 0.0;

void menuLoop() {
    menuEntry_s* me;
    menu_s* menu = menuGetCurrent();
    int i;
    int cnt=0;
    int x, y;

    for (x=0; x<1280; x++) {
        for (y=0; y<720; y++) {
            DrawPixelRaw(x, y, MakeColor(45, 55, 66, 255));
        }
    }

    // header labels
    DrawText(tahoma24, 64, 16, MakeColor(255, 255, 255, 255), "The Homebrew Launcher");
    DrawText(tahoma12, 64 + 256 + 128 + 128, 16 + 16, MakeColor(255, 255, 255, 255), "v1.0.0");
    DrawText(tahoma12, 64 + 256 + 128 + 128, 16 + 16 + 16, MakeColor(255, 255, 255, 255), menu->dirname);

    // animation state
    //state = (approxSin(timer*2.0)+1.0)/2.0;
    //state = 1.0;

    char buf[256];
    snprintf(buf, 256, "state: %f", state);
    DrawText(tahoma12, 64 + 256 + 128 + 128, 16 + 16 + 16 + 16, MakeColor(255, 255, 255, 255), buf);
    
    // waves
    drawWave(timer, MakeColor(73, 103, 169, 255), 100.0 + (180.0 * state), 0.0, 3.0);
    drawWave(timer, MakeColor(66, 154, 159, 255), 90.0 + (180.0 * state), 2.0, 3.5);
    drawWave(timer, MakeColor(96, 204, 204, 255), 80.0 + (180.0 * state), 4.0, -2.5);
    timer += 0.025;

    if(timer > lastBump + 4.0) {
	    lastBump = timer;
	    //targetScrollIdx+= 4;
	    targetState = 1 - targetState;
    }
    scrollIdx+= (targetScrollIdx-scrollIdx) * 0.09;
    state+= (targetState-state) * 0.09;
    
    
    if (menu->nEntries==0)
    {
	    // no entries
        DrawText(tahoma12, 64, 96 + 32, MakeColor(240, 240, 240, 255), textGetString(StrId_NoAppsFound_Msg));
    } else
    {

	    const int tileSize = 150;
	    const int tileSpacing = 64 - (int) (32.0 * state);
	    const int numTiles = 4;
	    
	    const int firstX = 1280/2 - (((numTiles-1) * tileSpacing) + (numTiles*tileSize))/2;

        // Draw menu entries
        for (me = menu->firstEntry, i = 0; me; me = me->next, i ++) {
	        int indivViewX = firstX + ((tileSize+tileSpacing)*(i - scrollIdx));
	        int indivViewY = 200 + (300.0 * state);
	        int gridViewX = firstX + ((tileSize+tileSpacing)*((int) (i - scrollIdx + numTiles) % numTiles));
	        if(i - scrollIdx < 0) {
		        indivViewX+= (int) (300.0*(1.0-state) * ((i - scrollIdx) + 1 - numTiles));
		        indivViewY = 500;
		        //drawEntry(me, cnt, i==menu->curEntry, gridViewX, 200 - tileSize - tileSpacing - (500.0 * state), tileSize);
	        }
	        if(i - scrollIdx >= numTiles) {
		        indivViewX+= (int) (300.0*(1.0-state) * ((i - scrollIdx) + 1 - numTiles));
		        indivViewY = 500;
		        if(i - scrollIdx < numTiles * 2) {
			        drawEntry(me, cnt, i==menu->curEntry, gridViewX, 200 + tileSize + tileSpacing + (500.0 * state), tileSize);
		        }
	        }
	        drawEntry(me, cnt, i==menu->curEntry, indivViewX, indivViewY, tileSize);

	        if(i == menu->curEntry) {
		        drawDetails(me);
	        }
	        
	        cnt++;
        }
    }
}
