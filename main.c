#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

// global variables (these might change later tho, not constant despite caps)

int GLOBAL_DEATH_AGE = 30;
int GLOBAL_BIRTH_AGE = 10;

enum Screen { START = 1, PAUSE, END, GAME };
enum Dissolution {
  NO_DISSOLUTION,
  ENVIRONMENTAL,
  BIOLOGICAL,
  MENTAL,
  TEMPORAL,
  SPIRITUAL
};

struct StartCircle {
  Vector2 pos;
  Vector2 direction;
};

void start_screen_animation(struct StartCircle *circles);
void game_screen();

struct Button {
  Image image;
  Texture2D texture;
  char text[30];
  Vector2 pos;
  Rectangle rect;
};

struct Player {
  int points;
  enum Dissolution active_dissolution;
};

struct Organism {
  int id;
  int parent_id;
  double time_until_next_birth;
  int age;
  Vector2 position;
  int radius;
  bool alive;
  Vector2 direction;
  double time_of_birth;
};

struct Button load_image_button(char *image_path, Vector2 pos) {
  struct Button btn = {LoadImage(image_path)};
  btn.pos = (Vector2){pos.x, pos.y};
  // this function causes a memory leak of about 11kb per function call, still
  // thinking of a solution
  ImageResizeNN(&btn.image, 60, 50);

  btn.texture = LoadTextureFromImage(btn.image);
  btn.rect =
      (Rectangle){btn.pos.x, btn.pos.y, btn.image.width, btn.image.height};

  return btn;
}

void handle_button_state(struct Button *btn, int id,
                         enum Dissolution current_dissolution,
                         enum Dissolution preferred_dissolution) {
  // to update the button position to always snap to the bottom os the screen
  // should probably move this to its own function

  btn->pos.y = GetScreenHeight() - 60;
  btn->rect.y = btn->pos.y;

  int center_x = GetScreenWidth() / 2;
  int spacing = btn->rect.width + 20;

  btn->pos.x = center_x + (id - 2) * spacing;
  btn->rect.x = btn->pos.x;

  if (CheckCollisionPointRec(GetMousePosition(), btn->rect)) {
    DrawTexture(btn->texture, btn->pos.x, btn->pos.y, GREEN);
    DrawRectangleLinesEx(btn->rect, 1, WHITE);
  } else if (current_dissolution == preferred_dissolution) {
    DrawTexture(btn->texture, btn->pos.x, btn->pos.y, GREEN);
  } else {

    DrawTexture(btn->texture, btn->pos.x, btn->pos.y, WHITE);
  }
}

struct Organism load_new_organism(int id, int parent_id, Vector2 position) {
  return (struct Organism){
      id,        parent_id,
      GetTime(), 0,
      position,  5,
      true,      (Vector2){GetRandomValue(-5, 5), GetRandomValue(-5, 5)},
      GetTime()};
};

struct OrganismAllocator {
  struct Organism *buffer;
  int capacity;
  int index;
  size_t index_size;
};

struct OrganismAllocator new_allocator(int index_size, int capacity) {
  return (struct OrganismAllocator){malloc(index_size * capacity), capacity, 0,
                                    index_size};
}

void append_organism_to_buffer(struct OrganismAllocator *allocator,
                               struct Organism organism) {
  allocator->buffer[allocator->index] = organism;
  allocator->index += 1;

  if (allocator->index > allocator->capacity - 1) {
    allocator->capacity *= 2;
    allocator->buffer =
        realloc(allocator->buffer, allocator->index_size * allocator->capacity);
  }
}

int main() {
  enum Screen current_screen = START;
  InitWindow(500, 500, "Art of being");
  SetTargetFPS(60);

  struct Player player = {100, NO_DISSOLUTION};
  struct StartCircle *circles = malloc(sizeof(struct StartCircle) * 10);

  struct OrganismAllocator organisms =
      new_allocator(sizeof(struct Organism), 50);

  struct Organism org_ =
      load_new_organism(organisms.index, organisms.index,
                        (Vector2){GetRandomValue(0, GetScreenWidth()),
                                  GetRandomValue(0, GetScreenHeight() - 80)});

  append_organism_to_buffer(&organisms, org_);
  struct Button environemental_dissolution_btn = load_image_button(
      "./assets/wormhole.png", (Vector2){50, GetScreenHeight() - 60});

  struct Button biological_dissolution_btn = load_image_button(
      "./assets/skull.png", (Vector2){150, GetScreenHeight() - 60});

  if (circles != NULL) {
    for (int i = 0; i < 10; i++) {
      Vector2 pos = (Vector2){GetRandomValue(0, GetScreenWidth()),
                              GetRandomValue(0, GetScreenHeight())};
      Vector2 dir = (Vector2){GetRandomValue(-2, 1), GetRandomValue(-2, 1)};

      circles[i] = (struct StartCircle){pos, dir};
    }
  }

  char points_text[20] = "Points:";
  char organisms_text[20] = "Active:";
  char mouse_hover_organism_text[50] = "";
  int existing_organisms = 0;
  while (!WindowShouldClose()) {
    float delta_time = GetFrameTime();
    BeginDrawing();

    ClearBackground(BLACK);
    switch (current_screen) {
    case GAME:

      sprintf(points_text, "Points: %i", player.points);
      sprintf(organisms_text, "Active: %i", existing_organisms);

      DrawText(points_text, 10, 10, 20, WHITE);
      DrawText(organisms_text, GetScreenWidth() - 150, 10, 20, WHITE);

      DrawLine(0, GetScreenHeight() - 80, GetScreenWidth(),
               GetScreenHeight() - 80, WHITE);

      handle_button_state(&environemental_dissolution_btn, 1,
                          player.active_dissolution, ENVIRONMENTAL);
      handle_button_state(&biological_dissolution_btn, 2,
                          player.active_dissolution, BIOLOGICAL);

      if (CheckCollisionPointRec(GetMousePosition(),
                                 environemental_dissolution_btn.rect) &&
          IsKeyDown(MOUSE_LEFT_BUTTON)) {
        player.active_dissolution = ENVIRONMENTAL;
      }

      if (CheckCollisionPointRec(GetMousePosition(),
                                 biological_dissolution_btn.rect) &&
          IsKeyDown(MOUSE_LEFT_BUTTON)) {
        player.active_dissolution = BIOLOGICAL;
      }

      if (IsKeyPressed(MOUSE_RIGHT_BUTTON) || IsKeyPressed(KEY_U)) {
        player.active_dissolution = NO_DISSOLUTION;
      }
      // DrawText(btn.text, 50, GetScreenHeight() -
      // 50, 15, WHITE);
      //

      // printf("%d\n", player.active_dissolution);
      existing_organisms = 0;
      if (organisms.buffer != NULL) {
        for (int i = 0; i < organisms.index; i++) {
          struct Organism organism = organisms.buffer[i];
          organism.age = GetTime() - organism.time_of_birth;
          if (organism.alive) {
            existing_organisms += 1;
            if (organism.age > GLOBAL_DEATH_AGE - 10) {
              DrawCircle(organism.position.x, organism.position.y,
                         organism.radius, DARKGRAY);
            } else {
              DrawCircle(organism.position.x, organism.position.y,
                         organism.radius, WHITE);
            }

            organism.position.x += organism.direction.x * delta_time;
            organism.position.y += organism.direction.y * delta_time;

            if (organism.position.x < 0 ||
                organism.position.x > GetScreenWidth()) {
              organism.direction.x *= -1;
            }
            if (organism.position.y < 0 ||
                organism.position.y > GetScreenHeight() - 80) {
              organism.direction.y *= -1;
            }
            if (organism.age > 30) {
              organism.alive = 0;
            }

            if (GetTime() - organism.time_until_next_birth > GLOBAL_BIRTH_AGE) {
              struct Organism org_ = (struct Organism)load_new_organism(
                  organisms.index, organism.id, organism.position);

              append_organism_to_buffer(&organisms, org_);

              organism.time_until_next_birth = GetTime();
            }

            if (organism.age > GLOBAL_DEATH_AGE) {
              organism.alive = false;
            }

            if (CheckCollisionPointCircle(organism.position, GetMousePosition(),
                                          organism.radius)) {
              sprintf(mouse_hover_organism_text, "organism %d\nage: %d",
                      organism.id, organism.age);
              DrawText(mouse_hover_organism_text,
                       GetMouseX() < GetScreenWidth() - 100 ? GetMouseX() + 30
                                                            : GetMouseX() - 100,
                       GetMouseY(), 15, WHITE);
            }

            organisms.buffer[i] = organism;
          }
        }
      }
      break;
    case PAUSE:
      break;
    case END:
      break;
    case START:
      start_screen_animation(circles);
      DrawText("Left Mouse Button to start", (GetScreenWidth() / 2) - 180,
               GetScreenHeight() / 2, 25, WHITE);
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        current_screen = GAME;
      }
      break;
    default:
      break;
    }

    EndDrawing();
  }

  CloseWindow();
  free(circles);
  free(organisms.buffer);
}

void start_screen_animation(struct StartCircle *circles) {
  for (int i = 0; i < 10; i++) {
    struct StartCircle circ = circles[i];
    circles[i].pos.x += circles[i].direction.x;
    circles[i].pos.y += circles[i].direction.y;

    if (circles[i].pos.x < 0 || circles[i].pos.x > GetScreenWidth()) {
      circles[i].direction.x *= -1;
    }
    if (circles[i].pos.y < 0 || circles[i].pos.y > GetScreenHeight()) {
      circles[i].direction.y *= -1;
    }
    DrawCircle(circles[i].pos.x, circles[i].pos.y, 10, WHITE);
  }
}
