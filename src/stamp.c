#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "stamp.h"

Stamp* new_polygon_stamp(Polygon* p) {
    Stamp* s = malloc(sizeof(Stamp));
    if (s == NULL) {
        return NULL;
    }

    s->polygon = p;
    s->tr_matrix = I;
    return s;
}

Stamp* new_rectangle_stamp(const size_t width, const size_t height) {
    Polygon* p = new_polygon();
    if (p == NULL) {
        return NULL;
    }

    add_vertex(p, (Point) {0, 0});
    add_vertex(p, (Point) {width, 0});
    add_vertex(p, (Point) {width, height});
    add_vertex(p, (Point) {0, height});

    Stamp* s = new_polygon_stamp(p);
    if (s == NULL) {
        free_polygon(p);
        return NULL;
    }

    return s;
}

Stamp* new_circle_stamp(const size_t steps, const size_t radius) {
    Polygon* p = new_polygon();
    if (p == NULL) {
        return NULL;
    }

    float stepa = (float) M_PI / (steps / 2.0f);
    for (size_t i = 0; i < steps; i++) {
        add_vertex(p, (Point) {
            cosf(stepa * i) * radius + radius,
            sinf(stepa * i) * radius + radius
        }); // +radius to move it to the positive side
    }

    Stamp* s = new_polygon_stamp(p);
    if (s == NULL) {
        free_polygon(p);
        return NULL;
    }

    return s;
}

void free_stamp(Stamp* s) {
    if (s->polygon) {
        free_polygon(s->polygon);
    }
    if (s->bitmap) {
        free_canvas(s->bitmap);
    }

    free(s);
}

void apply_matrix(Stamp* s) {
    if (s->polygon) {
        transform_polygon(s->polygon, s->tr_matrix);
    }

    s->tr_matrix = I;
}

void bresenham(Canvas* c, Color color, int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int incx = (x1 < x2) ? 1 : -1;
    int incy = (y1 < y2) ? 1 : -1;

    int err = ((dx > dy) ? dx : -dy) / 2;
    int e2 = 0;

    set_pixel(c, color, x1, y1);
    while(x1 != x2 && y1 != y2) {
        e2 = err;
        if (e2 >-dx) {
            err -= dy;
            x1 += incx;
        }
        if (e2 < dy) {
            err += dx;
            y1 += incy;
        }

        set_pixel(c, color, x1, y1);
    }
}

int draw_stamp_outline(Canvas* c, Color color, const Stamp* s) {
    if (s->type == BITMAP) {
        return -1;
    }

    Point from = transform_point(s->polygon->vertices[s->polygon->last - 1], s->tr_matrix);
    Point to = transform_point(s->polygon->vertices[0], s->tr_matrix);
    const int x2 = 10;
    bresenham(c, color, (int) from.x, (int) from.y, x2, (int) to.y);

    for (size_t i = 1; i < s->polygon->last; i++) {
        from = transform_point(s->polygon->vertices[i - 1], s->tr_matrix);
        to = transform_point(s->polygon->vertices[i], s->tr_matrix);
        bresenham(c, color, (int) from.x, (int) from.y, (int) to.x, (int) to.y);
    }

    return 1;
}

int fill_shape(Canvas* c, Color color, const Stamp* s);