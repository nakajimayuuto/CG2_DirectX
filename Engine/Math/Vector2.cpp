#include "Vector2.h"
#include <cmath>

Vector2 Vector2::operator+(const float& targetFloat){
    Vector2 result;
    result.x = x + targetFloat;
    result.y = y + targetFloat;
    return result;
}

Vector2 Vector2::operator-(const float& targetFloat){
    Vector2 result;
    result.x = x - targetFloat;
    result.y = y - targetFloat;
    return result;
}

Vector2 Vector2::operator*(const float& targetFloat){
    Vector2 result;
    result.x = x * targetFloat;
    result.y = y * targetFloat;
    return result;
}

Vector2 Vector2::operator/(const float& targetFloat){
    Vector2 result;
    result.x = x / targetFloat;
    result.y = y / targetFloat;
    return result;
}

Vector2 Vector2::operator=(const float& targetFloat){
    Vector2 result;
    result.x = targetFloat;
    result.y = targetFloat;
    return result;
}

Vector2& Vector2::operator+=(const float& targetFloat){
    x += targetFloat;
    y += targetFloat;
    return *this;
}

Vector2& Vector2::operator-=(const float& targetFloat){
    x -= targetFloat;
    y -= targetFloat;
    return *this;
}

Vector2& Vector2::operator*=(const float& targetFloat){
    x *= targetFloat;
    y *= targetFloat;
    return *this;
}

Vector2& Vector2::operator/=(const float& targetFloat){
    x /= targetFloat;
    y /= targetFloat;
    return *this;
}

Vector2 Vector2::operator+(const Vector2& targetVector2){
    Vector2 result;
    result.x = x + targetVector2.x;
    result.y = y + targetVector2.y;
    return result;
}

Vector2 Vector2::operator-(const Vector2& targetVector2) {
    Vector2 result;
    result.x = x - targetVector2.x;
    result.y = y - targetVector2.y;
    return result;
}

Vector2 Vector2::operator*(const Vector2& targetVector2) {
    Vector2 result;
    result.x = x * targetVector2.x;
    result.y = y * targetVector2.y;
    return result;
}

Vector2 Vector2::operator/(const Vector2& targetVector2) {
    Vector2 result;
    result.x = x / targetVector2.x;
    result.y = y / targetVector2.y;
    return result;
}

Vector2& Vector2::operator+=(const Vector2& targetVector2) {
    x += targetVector2.x;
    y += targetVector2.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& targetVector2) {
    x -= targetVector2.x;
    y -= targetVector2.y;
    return *this;
}

Vector2& Vector2::operator*=(const Vector2& targetVector2) {
    x *= targetVector2.x;
    y *= targetVector2.y;
    return *this;
}

Vector2& Vector2::operator/=(const Vector2& targetVector2) {
    x /= targetVector2.x;
    y /= targetVector2.y;
    return *this;
}


float Vector2::Length() {
    float length = sqrt(pow(x, 2.0f) + pow(y, 2.0f));

    return length;
};


Vector2 Vector2::Normalize() {
    float length = this->Length();
    Vector2 normalize;

    normalize.x = 0.0f;
    normalize.y = 0.0f;

    if (length != 0.0f) {
        normalize.x = x / length;
        normalize.y = y / length;
    }

    return normalize;
};