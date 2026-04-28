#include "Vector2.h"

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
