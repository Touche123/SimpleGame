#include "shapes.h"
#include "glad.h"

void DrawRectangle(Rectangle rec, vec2 origin, float rotation, color color)
{
    glbegin(GL_TRIANGLES);
    glColor4f(color.x, color.y, color.z, color.w);
    glEnd();
}