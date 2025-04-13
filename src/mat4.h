#ifndef MAT4_H
#define MAT4_H

#include <math.h>

#include "entity.h"

void mat4_identity(float *out) {
    for (int i = 0; i < 16; i++) out[i] = 0.0f;
    out[0] = out[5] = out[10] = out[15] = 1.0f;
}

void mat4_scale_in_place(float* m, float sx, float sy, float sz) {
    m[0] *= sx;  m[1] *= sx;  m[2] *= sx;  m[3] *= sx;   // Första kolumnen (X)
    m[4] *= sy;  m[5] *= sy;  m[6] *= sy;  m[7] *= sy;   // Andra kolumnen (Y)
    m[8] *= sz;  m[9] *= sz;  m[10]*= sz;  m[11]*= sz;   // Tredje kolumnen (Z)
    // Fjärde kolumnen (translation) lämnas orörd
}

void mat4_perspective(float *m, float fovy, float aspect, float znear, float zfar) {
    float f = 1.0f / tanf(fovy / 2.0f);
    float range_inv = 1.0f / (znear - zfar);

    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (zfar + znear) * range_inv;
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 2.0f * zfar * znear * range_inv;
    m[15] = 0.0f;
}

void mat4_translate(float *out, float x, float y, float z) {
    mat4_identity(out);
    out[12] = x;
    out[13] = y;
    out[14] = z;
}

void mat4_mul(float *out, float *a, float *b) {
    float res[16];
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            res[col + row * 4] =
                a[0 + row * 4] * b[col + 0] +
                a[1 + row * 4] * b[col + 4] +
                a[2 + row * 4] * b[col + 8] +
                a[3 + row * 4] * b[col + 12];
    for (int i = 0; i < 16; ++i) out[i] = res[i];
}

void mat4_rotate_y(float *out, float angle_rad) {
    mat4_identity(out);
    out[0] = cosf(angle_rad);
    out[2] = sinf(angle_rad);
    out[8] = -sinf(angle_rad);
    out[10] = cosf(angle_rad);
}

void mat4_rotate_x(float *out, float angle_rad) {
    mat4_identity(out);
    out[5] = cosf(angle_rad);
    out[6] = sinf(angle_rad);
    out[9] = -sinf(angle_rad);
    out[10] = cosf(angle_rad);
}

void mat4_rotate_z(float *out, float angle_rad) {
    mat4_identity(out);
    out[0] = cosf(angle_rad);
    out[1] = sinf(angle_rad);
    out[4] = -sinf(angle_rad);
    out[5] = cosf(angle_rad);
}

void mat4_rotate_xyz(float *out, float angle_x, float angle_y, float angle_z) {
    float rx[16], ry[16], rz[16], rxy[16];

    mat4_rotate_x(rx, angle_x);
    mat4_rotate_y(ry, angle_y);
    mat4_rotate_z(rz, angle_z);

    mat4_mul(rxy, ry, rx);   // först X, sen Y
    mat4_mul(out, rz, rxy);  // sen Z
}

void mat4_scale(float *out, float sx, float sy, float sz) {
    // Börja med identitetsmatris
    mat4_identity(out);

    out[0] = sx;   // [0][0]
    out[5] = sy;   // [1][1]
    out[10] = sz;  // [2][2]
}

void mat4_from_transform(float *out, const Transform *transform) {
    float T[16], R[16], S[16], RS[16];

    // Translation matrix
    mat4_translate(T, transform->position[0], transform->position[1], transform->position[2]);

    // Rotation matrix (assumes mat4_rotate_xyz exists and takes radians)
    mat4_rotate_xyz(R, transform->rotation[0], transform->rotation[1], transform->rotation[2]);

    // Scale matrix
    mat4_scale(S, transform->scale[0], transform->scale[1], transform->scale[2]);

    // Combine: model = T * R * S
    mat4_mul(RS, R, S);
    mat4_mul(out, T, RS);
}

void mat4_model_matrix(float *out, float tx, float ty, float tz, float angle_x, float angle_y, float angle_z) {
    float rotation[16], translation[16];

    // Beräkna rotationsmatrisen
    mat4_rotate_xyz(rotation, angle_x, angle_y, angle_z);

    // Beräkna translationsmatrisen
    mat4_identity(translation);
    translation[12] = tx;
    translation[13] = ty;
    translation[14] = tz;

    // Först multiplicera rotation med translation
    mat4_mul(out, translation, rotation);
}

void mat4_lookat(float *matrix, const float *eye, const float *center, const float *up) {
    float f[3], s[3], u[3];

    // f = normalize(center - eye)
    f[0] = center[0] - eye[0];
    f[1] = center[1] - eye[1];
    f[2] = center[2] - eye[2];
    vec3_normalize(f);

    // s = normalize(cross(f, up))
    vec3_cross(f, up, s);
    vec3_normalize(s);

    // u = cross(s, f)
    vec3_cross(s, f, u);

    // Bygg view-matris
    matrix[0] = s[0];
    matrix[1] = u[0];
    matrix[2] = -f[0];
    matrix[3] = 0.0f;

    matrix[4] = s[1];
    matrix[5] = u[1];
    matrix[6] = -f[1];
    matrix[7] = 0.0f;

    matrix[8] = s[2];
    matrix[9] = u[2];
    matrix[10] = -f[2];
    matrix[11] = 0.0f;

    matrix[12] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    matrix[13] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    matrix[14] = (f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2]);  // OBS: f är inverterad
    matrix[15] = 1.0f;
}

#endif
