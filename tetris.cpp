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
/* �� �κ��� �ҷ����� �׸� ����(block image)�� �ҷ����� ���� */
/*************************************************************/
//Parameter�� �Ѱ��� blockimage �ȿ� ���ο� �׸��� �Ҵ����ݴϴ�.
//path�� ���� �׸��� �޾ƿ��� �� �׸��� ���̿� ���̸� SDL_Rect ������ �޾ƿɴϴ�.
SDL_Texture* loadImage(std::string path, SDL_Rect* origin)
{
	//������ �ؽ��İ� ���������� ����� ���Դϴ�.
	SDL_Texture* newTexture = NULL;

	//��ο� �ִ� �׸��� �ҷ��ɴϴ�.
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

	//���������� ���������� ����� �ؽ��ĸ� blockimage �ȿ� �ִ� �ؽ��Ŀ� �Ѱ��ݴϴ�.
	return newTexture;
}




/******************************************************/
/* �� �κ��� �ҷ����� �ؽ���(block image)�� ���� ���� */
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

	bc_length//�� ������ ������ ��Ÿ���ش�.

};
typedef struct _blockimage
{
	//�׷����� �׸��� ����Ǵ� ��
	SDL_Texture* texture;

	//���� �̹����� ���� ���� (width, height, x, y)
	SDL_Rect origin;
	//���� �̹������� �߸� �簢���� ���� ����(width, height, x, y)
	//���⼭ x,y�� ���� �׸����� �簢���� ���۵Ǵ� ��ǥ�̸� width, height�� �߸� �簢�� ũ���.
	SDL_Rect clip;


} blockimage;
blockimage blockimageset[bc_length];
blockimage colorpattern; 


//Parameter�� �Ѱ��� blockimage �ȿ� �ִ� ������ �׸��� �Ҵ��� �������ݴϴ�.
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

//blockimage�� �ʱ�ȭ�Ѵ�
void blockimage_Init(blockimage* nowimage, SDL_Texture* texture, SDL_Rect origin, SDL_Rect clip)
{
	nowimage->texture = texture;
	nowimage->origin = origin;
	nowimage->clip = clip;
}

//blockimageset�� �ʱ�ȭ�Ѵ�.
bool blockimage_setInit()
{
	SDL_Rect neworigin;
	SDL_Rect newclip;
	SDL_Texture* newTexture;
	newTexture = loadImage("image/colorpattern.bmp", &neworigin);
	if (newTexture==NULL)
	{
		//�׸� �ҷ����� �� �����ϸ� return ���� false�� �־��ش�.
		return false;
	}
	else
	{
		//�׸� �ҷ����� �Ϳ� �����ϸ� sprite�� �й����ش�.
		for (int i = 0; i < 7; i++) //red -> orange -> green .. �̷� �������
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
		//�׸� �ҷ����� �� �����ϸ� return ���� false�� �־��ش�.
		return false;
	}
	else
	{
		//gray�� ���� �׸��� �ҷ��ͼ� �־��ش�.
		newclip.w = neworigin.w;
		newclip.h = neworigin.h;
		newclip.x = 0;
		newclip.y = 0;
		blockimage_Init(&blockimageset[bc_gray], newTexture, neworigin, newclip);
	}

	//������ ��������� ����
	return true;
}

const int boardwidth = 10;
const int boardheight = 25;

//blockimage�� �׸� �� ���� �Լ�
//�� �� x�� y�� �׷����� ��ġ�� ��ǥ
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
/* �� �κ��� ��Ʈ���� ����� �׷����� �ǿ� ���� ����. */
/******************************************************/

typedef struct _block
{
	/* �� �κ��� ó���� �ʱ�ȭ�� �� �� ����������.*/

	//block�� �׸� �׸� ��ġ�� �˷��ִ� �� ���δ�.
	//������ ��ǥ�� ��������� (0,0) �����̹Ƿ� �����ϴ��� ������ ����� ��ġ�� �°� �ٲ㼭 �������ش�.
	// �����ϴ� �������� (5,5)�� �ִ� ����� �׸��� �׷��� �� ��ġ�� (5,20) �����̴�. ��, 25�� ����� �ִ���̶�� ���� ���

	//!!!! ���� �׸� �׸� �� ����� ������ ���ӻ� �˰��򿡼� �Ű澲�� �ʴ´� !!!!
	int x;
	int y;

	//block�� �׷��ִ� �ؽ���(�׸� ����)
	blockimage* onimage; //Ȱ��ȭ ���� �� texture(�׶��׶� �ٲ��.)
	blockimage* offimage; //��Ȱ��ȭ ���� �� texture(�ٲ��� �ʴ´�.)

	//block�� ��ĥ�� ���� ������
	bool on;
	//block�� �� �ڸ��� �ִ��� ������.
	bool exist;

} block;

block board[boardheight][boardwidth];

/* �ʱ�ȭ �κ��� �׸� �ʱ�ȭ�� ���� �ǰ� ���� �����Ѵ�.*/
//block�� �ʱ�ȭ�� �� ���� �Լ�
void block_Init(block* nowblock, int x, int y, blockimage* onimage, blockimage* offimage)
{

	nowblock->x = x;
	nowblock->y = y;
	nowblock->onimage = onimage;
	nowblock->offimage = offimage;
	nowblock->on = false;
	nowblock->exist = false;
}
//board�� �ʱ�ȭ�� �� ���� �Լ�
void board_Init()
{
	blockimage* temp_onimage;
	blockimage* temp_offimage;
	int i, j;
	for (i = 0; i < boardheight; i++)
		for (j = 0; j < boardwidth; j++)
		{
			//���Ƿ� ���� ������ �����ش�.
			temp_onimage = &blockimageset[bc_red];
			temp_offimage = &blockimageset[bc_gray];
			//������ ��ǥ�� ��������� (0,0) �����̰� ����� ���� ��ġ�� �����ϴ��� �����̱� ������ [i][j]�� �׷����� �迭�� �°� ��ġ�� �ٲ��ش�.
			block_Init(&board[i][j], j, boardheight -1 - i, temp_onimage, temp_offimage);
		}
}

//block �ϳ��� �׸� �� ���� �Լ�
//�� �� x�� y�� �׷����� ��ġ�� ��ǥ
void block_render(block* nowblock)
{
	if (nowblock->on)
		blockimage_render(nowblock->onimage, nowblock->x, nowblock->y);
	else
		blockimage_render(nowblock->offimage, nowblock->x, nowblock->y);
}
//board ���θ� �׸� �� ���� �Լ�
void board_render()
{
	int i, j;
	for (i = 0; i < boardheight; i++)
		for (j = 0; j < boardwidth; j++)
			block_render(&board[i][j]);
}

//block�� ���� �Ҵ��� �����ϴ� �Լ�
void block_free(block* nowblock)
{
	nowblock->x = 0;
	nowblock->y = 0;
	blockimage_free(nowblock->onimage);
	blockimage_free(nowblock->offimage);
	nowblock->on = false;
	nowblock->exist = false;
}
//board�� ���� �Ҵ��� �����ϴ� �Լ�
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
/* ��Ʈ���� �˰��� ���� ���� �� ������ ����� �̵�, ȸ��, ����, �ʱ�ȭ�� ���� ����. */
/****************************************************************************************/

//��Ʈ���� ��Ͽ� ���� �迭.
//��� ����� mino��� �Ѵ�.
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
	},//�۴��, i�̳�
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//�׸�, o �̳�
	{
		{ 0, 0, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 0, 0 }
	},//Z�̳�
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 0 }
	},//S�̳�
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 1 },
		{ 0, 0, 0, 0 }
	},//J�̳�
	{
		{ 0, 0, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//L�̳�
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},//T�̳�
};

//���� �������� �ִ� ��� ����
typedef struct _infoblock
{
	bool mino[4][4];
	int color;

	//���� �ϴ��� (0,0)�̶�� �ϰ� 4x4 ��� ��ġ�� ��ü board���� ��� ��ġ�� �ִ��� �˷��ش�. 
	int x;
	int y;
} infoblock;
//���� ���� ���
infoblock nowblock;
//���� ������ Ű���峪 �̵����� ���� ��ġ�� �ӽ÷� ��� ���
infoblock postblock;
//���� ���� ���
infoblock nextblock;
//������ ��� ����

//��� ����
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
//��� ȸ��
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
//����� ������ ���� ����� ó�� ������ ���� �������� �ٸ��� ó�����ش�.
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

//��� �ʱ�ȭ
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
//ó�� ������ �� �������� ��� �ʱ�ȭ
void infoblock_setinit()
{
	infoblock_init(&nowblock);
	infoblock_init(&nextblock);
}

bool block_IsLocationOK(infoblock* next);
//ó�� ������ �� �������� ��� ����
//�� �� ó�� ����� �����Ǿ��� �� �ٸ� ��ϵ鿡 �ɷ� ������ �� ������ �����Ѵ�.
//����������, ���ӿ���������?
bool infoblock_setrenew()
{
	infoblock_copy(&nowblock, &nextblock);
	infoblock_init(&nextblock);
	if (!block_IsLocationOK(&nowblock))
		return false;
	return true;
}


/*****************************************************************************************/
/* ��Ʈ���� �˰��� ���� ���� �� ���� �������� ��ϰ� board�� ���õ� ����. */
/****************************************************************************************/

enum direction
{
	down,
	extremedown,
	left,
	right,
	rotate
};

//block�� ��� �������� �̵��ϰ��� �Ͽ��� �� �� �̵� �������� �̵��� �� �ִ��� ������ �����ִ� �Լ�
bool block_IsLocationOK(infoblock* next)
{
	int nextx, nexty;

	for(int i=0;i<4;i++)
		for (int j = 0; j < 4; j++)
		{
			//infoblock�� ����� ����� ����� �̵��Ϸ��� ���� �̹� �ִ� ��ϰ� ��ġ�ų� board ������ �Ѿ���� �ƴ��� Ȯ���Ѵ�.
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

//block�� �̵��ϰ��� �ϴ� �������� �̵����ִ� �Լ�.
//�̵��� �� �Ǹ� false, �̵��� �Ǹ� true�� return�Ѵ�.
//�̵����� ���� ����� ��ġ�� next�� �����Ѵ�. ����, ���� ���οʹ� ������� ����ȴ�.
bool block_goNextLocation(infoblock* now, infoblock* next, int orient)
{
	infoblock_copy(next, now);

	switch (orient)
	{
	case rotate:
		//rotate�� �켱���Ѻ���.
		infoblock_rotate(next);
		//ȸ���� �Ǹ� �����Ѵ�.
		if (!block_IsLocationOK(next))
			return false;
		break;

	case down :
		//�� ĭ ��������.
		next->y -= 1;
		//�� ĭ ���� �� ������, ���� ����� �� ĭ ���� ������� �����Ѵ�.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case left :
		//�� ĭ �������� ������.
		next->x -= 1;
		//�� ĭ �������� �� �� ������, ���� ����� �� ĭ �������� �� ������� �����Ѵ�.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case right :
		//�� ĭ ���������� ������.
		next->x += 1;
		//�� ĭ ���������� �� �� ������, ���� ����� �� ĭ ���������� �� ������� �����Ѵ�.
		if (!block_IsLocationOK(next))
			return false;
		break;
	case extremedown :
		//�� �������� ������ ��������.
		while (block_IsLocationOK(next))
			next->y -= 1;
		//ó������ �� ���������� ������ ���������Ƿ�, next.y�� ��ĭ ���� �ö󰡸� ���������� ���Ǵ� ������ ����.
		next->y += 1;
		break;
	}
	return true;
}

//�׸� �� �ִ� �� Ȯ���� �Ǹ� ������ board ���� �׷��ش�.
void block_updateLocation(infoblock* prev, infoblock* now)
{
	int x, y;

	//������ �׷����ִ� �� �����.
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

	//���� �׸� ���� �׸���.
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

	//�׷����� ���� �׷��� ����� ���� ����� �ȴ�.
	infoblock_copy(prev, now);
}

bool debugon[boardheight][boardwidth];
bool debugexist[boardheight][boardwidth];

//�� ĭ�� �� ä������ Ȯ�����ְ�, �� ĭ�� ä������ �ν��ش�.
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
		//�μ������� �� ĭ�� �����ش�.
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


		//�ٽ� ������ �ִ��� Ȯ���Ѵ�.
		block_checkCrush();
	}
}

//����� �� �̻� ������ �� ���� ���°� �Ǹ�, ���ο� ������� �ٲ��ֱ� ���� ���� ����� �� �ڸ��� ���������ش�.
void block_fixLocation(infoblock* now)
{
	int x, y;

	//����� ���� ��ġ�� ������Ų��.
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


//gameover�� �Ǿ����� �˷��ִ� ����
bool gameover = false;
//game over ȭ���� �����Ѵ�.
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
/*               SDL ��ü���� �ʱ�ȭ ����.            */
/******************************************************/

//SDL�� �����ϰ� window�� �����.
//�Ʒ� ������ �״�� �����Ѱ�.
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
//�׸� ������ �ҷ�����, �׸� ���Ͽ� �ش��ϴ� �迭�� �� �ʱ�ȭ���ش�.
bool initmedia()
{
	//blockimage���� �ҷ��ͼ� �ʱ�ȭ�Ѵ�.
	if (!blockimage_setInit())
		return false;

	//block �迭�� �ʱ�ȭ�Ѵ�.
	board_Init();

	// nowblock, nextblock�� �ʱ�ȭ�Ѵ�.
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

				//�׳� ����Ǵ� ��.
				switch (count++)
				{
				case GAMESPEED :
					count = 0;
					
					//������ �� ������ �ڵ����� �� ĭ�� �����ش�.
					if (block_goNextLocation(&nowblock, &postblock, down))
						block_updateLocation(&nowblock, &postblock);
					//������ �� ������ ������ �� �� �ִ��� �ľ��ϰ�, �������� �������ش�.
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