#include <SDL.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <SDL_image.h>

// Stałe
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 800;
const int RECT_SIZE = 30;
const int PLATFORM_WIDTH = 70;
const int PLATFORM_HEIGHT = 10;
const float GRAVITY = 0.01f;
const float JUMP_STRENGTH = -3.8f;
const float MOVE_SPEED = 1.0f;

// Struktura Platform
struct Platform
{
   SDL_Rect rect;
};

// Funkcja sprawdzająca kolizje
bool checkCollision(SDL_Rect a, SDL_Rect b)
{
   return SDL_HasIntersection(&a, &b);
}

int main(int argc, char *argv[])
{
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
      return -1;
   }

   if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
   {
      std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
      SDL_Quit();
      return -1;
   }

   SDL_Window *window = SDL_CreateWindow("Doodle Jump",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         WINDOW_WIDTH, WINDOW_HEIGHT,
                                         SDL_WINDOW_SHOWN);
   if (window == nullptr)
   {
      std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return -1;
   }

   SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
   if (renderer == nullptr)
   {
      std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   // Tło
   SDL_Texture *backgroundTexture = IMG_LoadTexture(renderer, "bg.png");
   if (backgroundTexture == nullptr)
   {
      std::cerr << "Unable to load background image! SDL_Error: " << SDL_GetError() << std::endl;
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   // Tekstura gracza
   SDL_Texture *playerTexture = IMG_LoadTexture(renderer, "ball.png");
   if (playerTexture == nullptr)
   {
      std::cerr << "Unable to load player image! SDL_Error: " << SDL_GetError() << std::endl;
      SDL_DestroyTexture(backgroundTexture);
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   // Tekstura platformy
   SDL_Texture *platformTexture = IMG_LoadTexture(renderer, "desk.png");
   if (platformTexture == nullptr)
   {
      std::cerr << "Unable to load platform image! SDL_Error: " << SDL_GetError() << std::endl;
      SDL_DestroyTexture(playerTexture);
      SDL_DestroyTexture(backgroundTexture);
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   SDL_Rect playerRect = {WINDOW_WIDTH / 2 - RECT_SIZE / 2, WINDOW_HEIGHT - RECT_SIZE - PLATFORM_HEIGHT, RECT_SIZE, RECT_SIZE};
   float velocityY = 0.0f;
   float velocityX = 0.0f;

   std::vector<Platform> platforms;
   srand(static_cast<unsigned int>(time(0)));

   Platform startingPlatform;
   startingPlatform.rect = {WINDOW_WIDTH / 2 - PLATFORM_WIDTH / 2, WINDOW_HEIGHT - PLATFORM_HEIGHT, PLATFORM_WIDTH, PLATFORM_HEIGHT};
   platforms.push_back(startingPlatform);

   bool quit = false;
   SDL_Event e;

   float cameraY = 0.0f;
   int lastPlatformHeight = WINDOW_HEIGHT;

   while (!quit)
   {
      while (SDL_PollEvent(&e) != 0)
      {
         if (e.type == SDL_QUIT)
         {
            quit = true;
         }
         else if (e.type == SDL_KEYDOWN)
         {
            if (e.key.keysym.sym == SDLK_LEFT)
            {
               velocityX = -MOVE_SPEED;
            }
            else if (e.key.keysym.sym == SDLK_RIGHT)
            {
               velocityX = MOVE_SPEED;
            }
         }
         else if (e.type == SDL_KEYUP)
         {
            if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT)
            {
               velocityX = 0;
            }
         }
      }

      velocityY += GRAVITY;

      playerRect.y += static_cast<int>(velocityY);
      playerRect.x += static_cast<int>(velocityX);

      // Kolizje z bokami
      if (playerRect.x < 0)
      {
         playerRect.x = 0;
      }
      if (playerRect.x + playerRect.w > WINDOW_WIDTH)
      {
         playerRect.x = WINDOW_WIDTH - playerRect.w;
      }

      // Kolizje z platformami
      bool onPlatform = false;
      for (auto &platform : platforms)
      {
         if (checkCollision(playerRect, platform.rect) && velocityY > 0)
         {
            playerRect.y = platform.rect.y - RECT_SIZE;
            velocityY = JUMP_STRENGTH;
            onPlatform = true;
         }
      }

      // Sprawdzenie, czy gracz nie spadł
      if (playerRect.y > WINDOW_HEIGHT)
      {
         quit = true;
      }

      // Kamera i platf
      if (playerRect.y < cameraY + WINDOW_HEIGHT / 2)
      {
         cameraY = playerRect.y - WINDOW_HEIGHT / 2;

         if (playerRect.y < cameraY + WINDOW_HEIGHT - 100)
         {
            int newPlatformY = lastPlatformHeight - (rand() % (WINDOW_HEIGHT / 4)) - PLATFORM_HEIGHT;
            Platform newPlatform;
            newPlatform.rect = {
                rand() % (WINDOW_WIDTH - PLATFORM_WIDTH),
                newPlatformY,
                PLATFORM_WIDTH,
                PLATFORM_HEIGHT};
            platforms.push_back(newPlatform);

            lastPlatformHeight = newPlatformY;
         }
      }

      auto it = platforms.begin();
      while (it != platforms.end())
      {
         if ((*it).rect.y > cameraY + WINDOW_HEIGHT + 250)
         {
            it = platforms.erase(it);
         }
         else
         {
            ++it;
         }
      }

      SDL_RenderClear(renderer);

      SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

      SDL_Rect adjustedPlayerRect = {playerRect.x, playerRect.y - static_cast<int>(cameraY), playerRect.w, playerRect.h};
      SDL_RenderCopy(renderer, playerTexture, NULL, &adjustedPlayerRect);

      for (auto &platform : platforms)
      {
         SDL_Rect adjustedPlatformRect = {platform.rect.x, platform.rect.y - static_cast<int>(cameraY), platform.rect.w, platform.rect.h};
         SDL_RenderCopy(renderer, platformTexture, NULL, &adjustedPlatformRect);
      }

      SDL_RenderPresent(renderer);
   }

   SDL_DestroyTexture(platformTexture);
   SDL_DestroyTexture(playerTexture);
   SDL_DestroyTexture(backgroundTexture);
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   IMG_Quit();
   SDL_Quit();

   return 0;
}
