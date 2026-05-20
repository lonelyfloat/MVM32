#include "utils.h"
#include <raylib/raymath.h>

Vector2 ProjectRectangleOnAxis(Rectangle r, Vector2 axis) {
    Vector2 pts[4];
    pts[0] = (Vector2){r.x,r.y};
    pts[1] = (Vector2){r.x+r.width,r.y};
    pts[2] = (Vector2){r.x+r.width,r.y+r.height};
    pts[3] = (Vector2){r.x,r.y+r.height};
    return ProjectPolygonOnAxis(pts,4,axis);
}

Vector2 ProjectPolygonOnAxis(Vector2* vertices, int vertexCount, Vector2 axis) {
    float min = Vector2DotProduct(axis, vertices[0]);
    float max = min;
    for(int i = 1; i < vertexCount; ++i) {
        float dot = Vector2DotProduct(axis, vertices[i]);
        if(dot < min) {
            min = dot;
        } else if(dot > max) {
            max = dot;
        }
    }
    return (Vector2){min,max};
}

static inline float max(float a, float b) {
    return a > b ? a : b;
}

static inline float min(float a, float b) {
    return a < b ? a : b;
}

float GetOverlap(Vector2 a, Vector2 b) {
    return max(0, min(a.y, b.y) - max(a.x, b.x));
}

float GetSignedOverlap(Vector2 moving, Vector2 still) {
    float movingHW = moving.x + moving.y/2;
    float stillHW = still.x + still.y/2;
    float sign = 1;
    if(movingHW < stillHW) sign = -1;
    return sign*GetOverlap(moving,still);
}

Vector2 ResolveRectStaticRect(Rectangle moving, Rectangle st) {
    Vector2 movingCenter = (Vector2){moving.x + (moving.width / 2), moving.y + (moving.height / 2)};
    Vector2 stCenter = (Vector2){st.x + (st.width / 2), st.y + (st.height / 2)};
    float xOverlap = (moving.width * 0.5) + (0.5*st.width) - fabs(movingCenter.x - stCenter.x);
    float yOverlap = (moving.height * 0.5) + (0.5*st.height) - fabs(movingCenter.y - stCenter.y);
    if(xOverlap <= 0 || yOverlap <= 0) return Vector2Zero();
    if(xOverlap < yOverlap) {
        return movingCenter.x < stCenter.x ? (Vector2){-xOverlap, 0} : (Vector2){xOverlap,0};
    } else { 
        return movingCenter.y < stCenter.y ? (Vector2){0,-yOverlap} : (Vector2){0, yOverlap};
    }
}

Vector2 ResolveCircleStaticRect(Vector2 center, float radius, Rectangle rec) {
    Vector2 recCenter = (Vector2){rec.x + (0.5*rec.width), rec.y + (0.5*rec.height)};
    Vector2 closest = Vector2Add(recCenter,
            Vector2Clamp(Vector2Subtract(center, recCenter), 
            (Vector2){-0.5*rec.width, -0.5*rec.height},
            (Vector2){0.5*rec.width, 0.5*rec.height})
        );
    Vector2 inter = Vector2Subtract(center, closest);
    if(Vector2LengthSqr(inter) > (radius * radius)) return Vector2Zero();
    return Vector2Subtract(Vector2Scale(Vector2Normalize(inter), radius),
            inter);
}


Vector2 ResolveRectStaticTriangle(Rectangle rec, Vector2 triStart, Vector2 triBounds) {
    Vector2 side = Vector2Normalize(triBounds);
    Vector2 proj = {0};
    Vector2 axes[3] = {(Vector2){0,1}, (Vector2){1, 0}, (Vector2){side.y, -side.x}};

    float overlap = 10000000;
    for(int i = 0; i < 3; ++i) {
        float dot = Vector2DotProduct(axes[i], (Vector2){rec.x,rec.y});
        float rMin = dot;
        float rMax = dot;
        dot = Vector2DotProduct(axes[i], (Vector2){rec.x + rec.width,rec.y});
        rMin = min(rMin, dot);
        rMax = max(rMax, dot);
        dot = Vector2DotProduct(axes[i], (Vector2){rec.x + rec.width,rec.y + rec.height});
        rMin = min(rMin, dot);
        rMax = max(rMax, dot);
        dot = Vector2DotProduct(axes[i], (Vector2){rec.x,rec.y + rec.height});
        rMin = min(rMin, dot);
        rMax = max(rMax, dot);

        dot = Vector2DotProduct(axes[i], triStart);
        float tMin = dot;
        float tMax = dot;
        dot = Vector2DotProduct(axes[i], (Vector2){triStart.x ,triStart.y + triBounds.y});
        tMin = min(tMin, dot);
        tMax = max(tMax, dot);
        dot = Vector2DotProduct(axes[i], (Vector2){triStart.x + triBounds.x,triStart.y + triBounds.y});
        tMin = min(tMin, dot);
        tMax = max(tMax, dot);

        float curOverlap = max(0, min(rMax, tMax) - max(rMin, tMin));

        if(curOverlap < overlap) {
            bool negateAxis = Vector2DotProduct(axes[i], Vector2Subtract((Vector2){rec.x, rec.y},triStart)) < 0;
            overlap = curOverlap;
            proj = Vector2Scale(axes[i], negateAxis?-overlap:overlap);
        }
        if(curOverlap <= 0) break;
    }

    return proj;
}
