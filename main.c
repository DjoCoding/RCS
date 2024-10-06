#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

#define VEC2_IMPLEMENTATION
#include "./vec2.h"

#define min(a, b) ((a) < (b) ? (a) : (b)) 

typedef struct {
    int wh;      // window height
    int ww;      // window height
    char *wt;    // window title
    Color bg;    // the default background color
} App;

App app_create(int wh, int ww, char *wt) {
    App app = {0};
    app.wh = wh;
    app.ww = ww;
    app.wt = wt;
    return app;
}

void app_set_bgcolor(App *app, Color bg) {
    app->bg = bg;
}

void app_init(App *app) {
    InitWindow(app->ww, app->wh, app->wt);
}

int app_should_stop() {
    return WindowShouldClose();
}

void app_clear(App *app) {
    ClearBackground(app->bg);
}

void app_close(void) {
    CloseWindow();
}

typedef struct {
    fV2   c;    // center
    float b;    // border
    float r;    // radius
} Circle;

Circle circle_init(fV2 center, float r, float b) {
    Circle c = {0};
    c.c = center;
    c.b = b;
    c.r = r;
    return c;
}

void circle_render(Circle *c, Color circle_color, Color border_color) {
    /// outer circle rendering
    DrawCircle((int) c->c.x, (int) c->c.y, c->r + c->b, border_color);
    
    /// inner circle rendering
    DrawCircle((int) c->c.x, (int) c->c.y, c->r, circle_color);                          
}

#define TO_RADIANS(x)  ((x * PI) / 180)

void circle_rotate(Circle *c, fV2 center, float angle) {
    // bring the vector to the origin
    c->c = fv2_sub(c->c, center);

    // perform the rotation
    float x =  cos(TO_RADIANS(angle)) * c->c.x - sin(TO_RADIANS(angle)) * c->c.y;
    float y =  sin(TO_RADIANS(angle)) * c->c.x + cos(TO_RADIANS(angle)) * c->c.y;
    
    // bring back the vector to its original position
    c->c = fv2_add(fv2(x, y), center);
}

void circle_move(Circle *c, fV2 dc) {
    c->c = fv2_add(c->c, dc);
}

void circle_dump(Circle c) {
    printf("center =" FV2_FMT "\n", FV2_ARG(c.c));
}

float next_radius(float *current_radius) {
    *current_radius = *current_radius / 2;
    return *current_radius;
}

#define CIRCLE_COUNT 5

typedef struct {
    Circle cs[CIRCLE_COUNT];
} System;

#define CENTER_CIRCLE_RADIUS 3.0f
#define CIRCLE_INNER_COLOR  GRAY 
#define CIRCLE_BORDER_COLOR WHITE
#define CIRCLE_BORDER_SIZE  1.0f
#define CIRCLE_INITIAL_RADIUS (float) (min(WINDOW_HEIGHT, WINDOW_WIDTH) / 4)
#define CIRCLE_INITIAL_CENTER (fV2) { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 }

System system_init(float initial_radius, fV2 initial_center) {
    System s = {0};

    float angle = 0;

    // construct the first circle
    s.cs[0] = circle_init(initial_center, initial_radius, CIRCLE_BORDER_SIZE);

    for(size_t i = 1; i < CIRCLE_COUNT; ++i) {
        float r = next_radius(&initial_radius);
        fV2 center = fv2(s.cs[i - 1].c.x + ((int) s.cs[i - 1].r - r) * cosf(TO_RADIANS(angle)), s.cs[i - 1].c.y + ((int) s.cs[i - 1].r - r) * sinf(TO_RADIANS(angle)));
        s.cs[i] = circle_init(center, r, CIRCLE_BORDER_SIZE);
        angle += (float) (rand() % 360);
    } 

    return s;
}

void system_rotate(System *s, float angle) {
    for(size_t i = 1; i < CIRCLE_COUNT; ++i) {
        fV2 current = s->cs[i].c;
        circle_rotate(&s->cs[i], s->cs[i - 1].c, i * angle);        
        fV2 now = s->cs[i].c;

        fV2 dc = fv2_sub(now, current); 
        for(size_t j = i + 1; j < CIRCLE_COUNT; ++j) {
            circle_move(&s->cs[j], dc);
        }    
    }
}

void system_render(System *s) {
    for(size_t i = 0; i < CIRCLE_COUNT; ++i) {
        circle_render(&s->cs[i], CIRCLE_INNER_COLOR, CIRCLE_BORDER_COLOR);
    }

    for(size_t i = 1; i < CIRCLE_COUNT; ++i) {
        fV2 prev = s->cs[i - 1].c;
        fV2 current = s->cs[i].c;
        DrawLine(prev.x, prev.y, current.x, current.y, BLACK );
        DrawCircle(prev.x, prev.y, CENTER_CIRCLE_RADIUS, RED);
    }

    DrawCircle(s->cs[CIRCLE_COUNT - 1].c.x, s->cs[CIRCLE_COUNT - 1].c.y, CENTER_CIRCLE_RADIUS, RED);
}

void system_dump(System *s) {
    for(size_t i = 0; i < CIRCLE_COUNT; ++i) {
        circle_dump(s->cs[i]);
    }

    for(size_t i = 0; i < 10; ++i) {
        printf("-");
    }
    printf("\n");
}


#define WINDOW_HEIGHT 768
#define WINDOW_WIDTH  1366


int main(void) {
    App app = app_create(WINDOW_HEIGHT, WINDOW_WIDTH, "circles");

    app_init(&app);
    app_set_bgcolor(&app, BLACK);

    float angle = 1.0f;
    
    System s = system_init(CIRCLE_INITIAL_RADIUS, CIRCLE_INITIAL_CENTER);

    SetTargetFPS(60);

    while(!app_should_stop()) {
        system_rotate(&s, angle);

        BeginDrawing();
        {
            app_clear(&app);
            system_render(&s);
        }
        EndDrawing();
    }

    app_close();
    return 0;
}