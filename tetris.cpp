#include<stdio.h>
#include<time.h>

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//gamespeed
#define GAMESPEED 10

//Screen dimension constants
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;


/*************************************************************/
/* 이 부분은 불러오는 그림 파일(block image)을 불러오는 내용 */
/*************************************************************/
//Parameter로 넘겨준 blockimage 안에 새로운 그림을 할당해줍니다.
//path를 통해 그림을 받아오고 그 그림의 넓이와 높이를 SDL_Rect 변수에 받아옵니다.
SDL_Texture* loadImage(std::string path, SDL_Rect* origin)
{
	//저장할 텍스쳐가 마지막으로 저장될 곳입니다.
	SDL_Texture* newTexture = NULL;

	//경로에 있는 그림을 불러옵니다.
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		return false;
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			return NULL;
		}
		else
		{
			//Get image dimensions
			origin->x = 0;
			origin->y = 0;
			origin->w = loadedSurface->w;
			origin->h = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//성공했으면 마지막으로 저장된 텍스쳐를 blockimage 안에 있는 텍스쳐에 넘겨줍니다.
	return newTexture;
}




/******************************************************/
/* 이 부분은 불러오는 텍스쳐(block image)에 대한 내용 */
/******************************************************/

enum blockcolor
{
	bc_red,
	bc_orange,
	bc_green,
	bc_yellow,
	bc_blue,
	bc_indigo,
	bc_purple,
	bc_gray,

	bc_length//총 색깔의 개수를 나타내준다.

};
typedef struct _blockimage
{
	//그려지는 그림이 저장되는 곳
	SDL_Texture* texture;

	//원래 이미지에 대한 정보 (width, height, x, y)
	SDL_Rect origin;
	//원래 이미지에서 잘릴 사각형에 대한 정보(width, height, x, y)
	//여기서 x,y는 원래 그림에서 사각형이 시작되는 좌표이며 width, height는 잘릴 사각형 크기다.
	SDL_Rect clip;


} blockimage;
blockimage blockimageset[bc_length];
blockimage colorpattern; 


//Parameter로 넘겨준 blockimage 안에 있는 내용의 그림을 할당을 해제해줍니다.
bool blockimage_free(blockimage* nowimage)
{
	if (nowimage->texture != NULL)
	{
		SDL_DestroyTexture(nowimage->texture);
		nowimage->texture = NULL;
		nowimage->origin.x = 0;
		nowimage->origin.y = 0;
		nowimage->origin.w = 0;
		nowimage->origin.h = 0;
		nowimage->clip.x = 0;
		nowimage->clip.y = 0;
		nowimage->clip.w = 0;
		nowimage->clip.h = 0;
		return true;
	}
	return false;
}

//blockimage를 초기화한다
void blockimage_Init(blockimage* nowimage, SDL_Texture* texture, SDL_Rect origin, SDL_Rect clip)
{
	nowimage->texture = texture;
	nowimage->origin = origin;
	nowimage->clip = clip;
}

//blockimageset을 초기화한다.
bool blockimage_setInit()
{
	SDL_Rect neworigin;
	SDL_Rect newclip;
	SDL_Texture* newTexture;
	newTexture = loadImage("image/colorpattern.bmp", &neworigin);
	if (newTexture==NULL)
	{
		//그림 불러오는 거 실패하면 return 값에 false를 넣어준다.
		return false;
	}
	else
	{
		//그림 불러오는 것에 성공하면 sprite를 분배해준다.
		for (int i = 0; i < 7; i++) //red -> orange -> green .. 이런 순서대로
		{
			newclip.w = neworigin.w / 7;
			newclip.h = neworigin.h;
			newclip.x = i * newclip.w;
			newclip.y = 0;	
			blockimage_Init(&blockimageset[i], newTexture, neworigin, newclip);
		}
	}

	newTexture = loadImage("image/graypattern.bmp", &neworigin);
	if (newTexture == NULL)
	{
		//그림 불러오는 거 실패하면 return 값에 false를 넣어준다.
		return false;
	}
	else
	{
		//gray일 때의 그림을 불러와서 넣어준다.
		newclip.w = neworigin.w;
		newclip.h = neworigin.h;
		newclip.x = 0;
		newclip.y = 0;
		blockimage_Init(&blockimageset[bc_gray], newTexture, neworigin, newclip);
	}

	//무사히 통과했으면 성공
	return true;
}

const int boardwidth = 10;
const int boardheight = 25;

//blockimage를 그릴 때 쓰는 함수
//이 때 x와 y는 그려지는 위치의 좌표
void blockimage_render(blockimage* nowimage, int x, int y)
{
	//Set rendering space and render to screen
	// nowimage->clip.w * 
	// nowimage->clip.h 
	int optimized_width = SCREEN_WIDTH / boardwidth;
	int optimized_height = SCREEN_HEIGHT / boardheight;
	int optimized_x = x * optimized_width;
	int optimized_y = y * optimized_height;
	SDL_Rect renderQuad = { optimized_x, optimized_y , optimized_width, optimized_height };


	SDL_Rect renderClip = nowimage->clip;
	
	//Render to screen
	SDL_RenderCopy(gRenderer, nowimage->texture, &renderClip, &renderQuad);
}


/******************************************************/
/* 이 부분은 테트리스 블록이 그려지는 판에 대한 내용. */
/******************************************************/

typedef struct _block
{
	/* 이 부분은 처음에 초기화할 때 다 지정해주자.*/

	//block의 그림 그릴 위치를 알려주는 데 쓰인다.
	//윈도우 좌표로 좌측상단이 (0,0) 기준이므로 좌측하단이 기준인 블록의 위치에 맞게 바꿔서 지정해준다.
	// 좌측하단 기준으로 (5,5)에 있는 블록은 그림이 그려질 때 위치는 (5,20) 정도이다. 단, 25가 블록의 최대높이라고 했을 경우

	//!!!! 따라서 그림 그릴 때 빼고는 실제로 게임상 알고리즘에서 신경쓰지 않는다 !!!!
	int x;
	int y;

	//block이 그려주는 텍스쳐(그림 파일)
	blockimage* onimage; //활성화 됐을 때 texture(그때그때 바뀐다.)
	blockimage* offimage; //비활성화 됐을 때 texture(바뀌지 않는다.)

	//block을 색칠할 건지 말건지
	bool on;
	//block이 그 자리에 있는지 없는지.
	bool exist;

} block;

block board[boardheight][boardwidth];

/* 초기화 부분은 그림 초기화가 먼저 되고 나서 수행한다.*/
//block을 초기화할 때 쓰는 함수
void block_Init(block* nowblock, int x, int y, blockimage* onimage, blockimage* offimage)
{

	nowblock->x = x;
	nowblock->y = y;
	nowblock->onimage = onimage;
	nowblock->offimage = offimage;
	nowblock->on = false;
	nowblock->exist = false;
}
//board를 초기화할 때 쓰는 함수
void board_Init()
{
	blockimage* temp_onimage;
	blockimage* temp_offimage;
	int i, j;
	for (i = 0; i < boardheight; i++)
		for (j = 0; j < boardwidth; j++)
		{
			//임의로 빨간 색으로 정해준다.
			temp_onimage = &blockimageset[bc_red];
			temp_offimage = &blockimageset[bc_gray];
			//윈도우 좌표로 좌측상단이 (0,0) 기준이고 블록의 실제 위치는 좌측하단이 기준이기 때문에 [i][j]에 그려지는 배열에 맞게 위치를 바꿔준다.
			block_Init(&board[i][j], j, boardheight -1 - i, temp_onimage, temp_offimage);
		}
}

//block 하나를 그릴 때 쓰는 함수
//이 때 x와 y는 그려지는 위치의 좌표
void block_render(block* nowblock)
{
	if (nowblock->on)
		blockimage_render(nowblock->onimage, nowblock->x, nowblock->y);
	else
		blockimage_render(nowblock->offimage, nowblock->x, nowblock->y);
}
//board 전부를 그릴 때 쓰는 함수
void board_render()
{
	int i, j;
	for (i = 0; i < boardheight; i++)
		for (j = 0; j < boardwidth; j++)
			block_render(&board[i][j]);
}

//block에 대한 할당을 해제하는 함수
void block_free(block* nowblock)
{
	nowblock->x = 0;
	nowblock->y = 0;
	blockimage_free(nowblock->onimage);
	blockimage_free(nowblock->offimage);
	nowblock->on = false;
	nowblock->exist = false;
}
//board에 대한 할당을 해제하는 함수
void board_free()
{
	int i, j;
	for (i = 0; i < boardheight; i++)
		for (j = 0; j < boardwidth; j++)
		{
			block_free(&board[i][j]);
		}
}



/*****************************************************************************************/
/* 테트리스 알고리즘에 대한 내용 중 간단한 블록의 이동, 회전, 복사, 초기화에 대한 내용. */
/****************************************************************************************/

//테트리스 블록에 대한 배열.
//블록 모양을 mino라고 한다.
enum minospecies
{
	mino_i,
	mino_o,
	mino_z,
	mino_s,
	mino_j,
	mino_l,
	mino_t,
	mino_length
};
const bool mino[7][4][4] =
{
	{
		{ 0, 0, 1, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 1, 0}
	},//작대기, i미노
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//네모, o 미노
	{
		{ 0, 0, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 0, 0 }
	},//Z미노
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 0 }
	},//S미노
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 1 },
		{ 0, 0, 0, 0 }
	},//J미노
	{
		{ 0, 0, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//L미노
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//T미노
};

//현재 내려오고 있는 블록 정보
typedef struct _infoblock
{
	bool mino[4][4];
	int color;

	//좌측 하단을 (0,0)이라고 하고 4x4 블록 위치가 전체 board에서 어느 위치에 있는지 알려준다. 
	int x;
	int y;
} infoblock;
//현재 순번 블록
infoblock nowblock;
//현재 순번이 키보드나 이동했을 때의 위치를 임시로 담는 블록
infoblock postblock;
//다음 순번 블록
infoblock nextblock;
//내려올 블록 정보

//블록 복사
void infoblock_copy(infoblock* dest, infoblock* src)
{
	for(int i=0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			dest->mino[i][j] = src->mino[i][j];
		}
	dest->color = src->color;
	dest->x = src->x;
	dest->y = src->y;
}
//블록 회전
void infoblock_rotate(infoblock* dest)
{
	infoblock temp;
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			temp.mino[3-j][i] = dest->mino[i][j];
		}
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			dest->mino[i][j] = temp.mino[i][j];
		}
}
//블록의 종류에 따라서 블록이 처음 내려올 때의 시작점을 다르게 처리해준다.
void infoblock_initstarting(infoblock* dest, int seed)
{
	switch (seed)
	{
	case mino_i: 
		dest->y = boardheight-4;
		dest->x = boardwidth/2 - 2;
		break;
	case mino_o:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	case mino_z:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	case mino_s:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	case mino_j:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	case mino_l:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	case mino_t:
		dest->y = boardheight - 3;
		dest->x = boardwidth / 2 - 2;
		break;
	}
}

//블록 초기화
void infoblock_init(infoblock* dest)
{
	srand(time(NULL));
	int minoseed = rand() % mino_length;
	int colorseed = rand() % (bc_length -1);
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			dest->mino[i][j] = mino[minoseed][i][j];
		}
	dest->color = colorseed;
	infoblock_initstarting(dest, minoseed);
}
//처음 시작할 때 내려오는 블록 초기화
void infoblock_setinit()
{
	infoblock_init(&nowblock);
	infoblock_init(&nextblock);
}

bool block_IsLocationOK(infoblock* next);
//처음 시작할 때 내려오는 블록 갱신
//이 때 처음 블록이 생성되었을 때 다른 블록들에 걸려 생성될 수 없으면 실패한다.
//실패했으면, 게임오버겠지유?
bool infoblock_setrenew()
{
	infoblock_copy(&nowblock, &nextblock);
	infoblock_init(&nextblock);
	if (!block_IsLocationOK(&nowblock))
		return false;
	return true;
}


/*****************************************************************************************/
/* 테트리스 알고리즘에 대한 내용 중 현재 내려야할 블록과 board와 관련된 내용. */
/****************************************************************************************/

enum direction
{
	down,
	extremedown,
	left,
	right,
	rotate
};

//block이 어느 방향으로 이동하고자 하였을 때 그 이동 방향으로 이동할 수 있는지 없는지 따져주는 함수
bool block_IsLocationOK(infoblock* next)
{
	int nextx, nexty;

	for(int i=0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			//infoblock에 저장된 블록의 모양이 이동하려는 곳의 이미 있는 블록과 겹치거나 board 범위를 넘어갔는지 아닌지 확인한다.
			if (next->mino[j][i] == true)
			{
				nextx = next->x + i;
				nexty = next->y + j;
				if (0 <= nextx && nextx < boardwidth && 0 <= nexty && nexty < boardheight)
				{
					if (board[nexty][nextx].exist == true)
					{
						return false;
					}
				}
				else
					return false;
			}
		}
	return true;
}

//block이 이동하고자 하는 방향으로 이동해주는 함수.
//이동이 안 되면 false, 이동이 되면 true를 return한다.
//이동했을 때의 블록의 위치를 next에 저장한다. 성공, 실패 여부와는 관계없이 저장된다.
bool block_goNextLocation(infoblock* now, infoblock* next, int orient)
{
	infoblock_copy(next, now);

	switch (orient)
	{
	case rotate:
		//rotate를 우선시켜본다.
		infoblock_rotate(next);
		//회전이 되면 고정한다.
		if (!block_IsLocationOK(next))
			return false;
		break;

	case down :
		//한 칸 내려본다.
		next->y -= 1;
		//한 칸 내릴 수 있으면, 현재 블록을 한 칸 내린 블록으로 고정한다.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case left :
		//한 칸 왼쪽으로 가본다.
		next->x -= 1;
		//한 칸 왼쪽으로 갈 수 있으면, 현재 블록을 한 칸 왼쪽으로 간 블록으로 고정한다.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case right :
		//한 칸 오른쪽으로 가본다.
		next->x += 1;
		//한 칸 오른쪽으로 갈 수 있으면, 현재 블록을 한 칸 오른쪽으로 간 블록으로 고정한다.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case extremedown :
		//못 내려가질 때까지 내려본다.
		while (block_IsLocationOK(next))
			next->y -= 1;
		//처음으로 안 내려가지는 데까지 내려갔으므로, next.y를 한칸 위로 올라가면 마지막으로 허용되는 곳까지 간다.
		next->y += 1;
		break;
	}
	return true;
}

//그릴 수 있는 게 확인이 되면 실제로 board 내에 그려준다.
void block_updateLocation(infoblock* prev, infoblock* now)
{
	int x, y;

	//기존에 그려져있던 걸 지운다.
	for(int i=0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			if (prev->mino[j][i] == true)
			{
				x = prev->x + i;
				y = prev->y + j;
				if (0 <= x && x < boardwidth && 0 <= y && y < boardheight)
				{
					board[y][x].on = false;
				}
			}
		}

	//새로 그릴 것을 그린다.
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			if (now->mino[j][i] == true)
			{
				x = now->x + i;
				y = now->y + j;
				if (0 <= x && x < boardwidth && 0 <= y && y < boardheight)
				{
					board[y][x].onimage = &blockimageset[now->color];
					board[y][x].on = true;
				}
			}
		}

	//그렸으니 이제 그려준 블록이 현재 블록이 된다.
	infoblock_copy(prev, now);
}

bool debugon[boardheight][boardwidth];
bool debugexist[boardheight][boardwidth];

//한 칸을 꽉 채웠는지 확인해주고, 한 칸을 채웠으면 부숴준다.
void block_checkCrush()
{
	bool crushed = false;
	int i, j;
	for (i = 0; i < boardheight; i++)
	{
		for (j = 0; j < boardwidth; j++)
		{
			if (board[i][j].exist == false)
				break;
		}
		if (j == boardwidth)
		{
			crushed = true;
			break;
		}
	}
	if (crushed)
	{
		//부서졌으면 한 칸씩 내려준다.
		for (i +=1;i < boardheight; i++)
		{
			for (j = 0; j < boardwidth; j++)
			{
				board[i-1][j].on = board[i][j].on;
				board[i - 1][j].exist = board[i][j].exist;
				board[i - 1][j].onimage = board[i][j].onimage;
			}
		}
		for (j = 0; j < boardwidth; j++)
		{
			board[boardheight-1][j].on = false;
			board[boardheight - 1][j].exist = false;
		}

		for(int i=0;i<boardheight;i++)
			for (int j = 0; j < boardwidth; j++)
			{
				debugon[i][j] = board[i][j].on;
				debugexist[i][j] = board[i][j].exist;
			}


		//다시 깨진게 있는지 확인한다.
		block_checkCrush();
	}
}

//블록이 더 이상 움직일 수 없는 상태가 되면, 새로운 블록으로 바꿔주기 위해 현재 블록은 그 자리에 고정시켜준다.
void block_fixLocation(infoblock* now)
{
	int x, y;

	//블록을 현재 위치에 고정시킨다.
	for (int i = 0; i<4; i++)
		for (int j = 0; j < 4; j++)
		{
			if (now->mino[j][i] == true)
			{
				x = now->x + i;
				y = now->y + j;
				if (0 <= x && x < boardwidth && 0 <= y && y < boardheight)
				{
					board[y][x].exist = true;
				}
			}
		}
}


//gameover가 되었음을 알려주는 변수
bool gameover = false;
//game over 화면을 수행한다.
void isgameover()
{
	int i, j;
	gameover = true;
	for(i=0;i<boardheight;i++)
		for (j = 0; j < boardwidth; j++)
		{
			board[i][j].on = true;
			board[i][j].exist = false;
			board[i][j].onimage = &blockimageset[bc_red];
		}
}

/******************************************************/
/*               SDL 전체적인 초기화 내용.            */
/******************************************************/

//SDL을 시작하고 window를 만든다.
//아래 내용은 그대로 복붙한거.
bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}
//그림 파일을 불러오고, 그림 파일에 해당하는 배열을 다 초기화해준다.
bool initmedia()
{
	//blockimage들을 불러와서 초기화한다.
	if (!blockimage_setInit())
		return false;

	//block 배열을 초기화한다.
	board_Init();

	// nowblock, nextblock을 초기화한다.
	infoblock_setinit();
	block_updateLocation(&nowblock, &nowblock);

	return true;

}

//Frees media and shuts down SDL
void close()
{
	board_free();
}


int main(int argc, char* args[])
{
	int count = 0;
	gameover = false;

	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!initmedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{
				if (gameover)
					continue;

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					else if (e.type == SDL_KEYDOWN)
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_UP:
								if (block_goNextLocation(&nowblock, &postblock, rotate))
									block_updateLocation(&nowblock, &postblock);
								break;
							case SDLK_DOWN:
								if (block_goNextLocation(&nowblock, &postblock, down))
									block_updateLocation(&nowblock, &postblock);
								break;
							case SDLK_LEFT:
								if (block_goNextLocation(&nowblock, &postblock, left))
									block_updateLocation(&nowblock, &postblock);
								break;
							case SDLK_RIGHT:
								if (block_goNextLocation(&nowblock, &postblock, right))
									block_updateLocation(&nowblock, &postblock);
								break;
							case SDLK_SPACE:
								if (block_goNextLocation(&nowblock, &postblock, extremedown))
									block_updateLocation(&nowblock, &postblock);
								break;
						}
					}

				
				}

				//그냥 실행되는 것.
				switch (count++)
				{
				case GAMESPEED :
					count = 0;
					
					//내려갈 수 있으면 자동으로 한 칸씩 내려준다.
					if (block_goNextLocation(&nowblock, &postblock, down))
						block_updateLocation(&nowblock, &postblock);
					//내려갈 수 없으면 점수가 날 수 있는지 파악하고, 다음으로 갱신해준다.
					else
					{
						block_fixLocation(&nowblock);
						block_checkCrush();
						if (!infoblock_setrenew())
							isgameover();
						else
							block_updateLocation(&nowblock, &nowblock);
					}
					
					break;
				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render objects
				board_render();

				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}