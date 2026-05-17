#pragma once

#include <cmath>
#include <numbers>
#include "Vector2.h"
#include "Vector3.h"

/// <summary>
/// DegreeをRadianに変換する
/// </summary>
/// <param name="degree">変換するDegree</param>
/// <returns></returns>
float Radian(float degree);

/// <summary>
/// RadianをDegreeに変換する
/// </summary>
/// <param name="radian">変換するRadian</param>
/// <returns></returns>
float Degree(float radian);

/// <summary>
/// DegreeをRadianに変換する
/// </summary>
/// <param name="degree">変換するDegree</param>
/// <returns></returns>
Vector3 Radian(Vector3 degree);

/// <summary>
/// RadianをDegreeに変換する
/// </summary>
/// <param name="radian">変換するRadian</param>
/// <returns></returns>
Vector3 Degree(Vector3 radian);

/// <summary>
/// 値を特定の範囲に収める
/// </summary>
/// <param name="clamping">収める値</param>
/// <param name="min">収める範囲の最小値</param>
/// <param name="max">収める範囲の最大値</param>
/// <returns></returns>
float Clamp(float clamping, float min, float max);

Vector3 Clamp(Vector3 clamping, Vector3 min, Vector3 max);

/// <summary>
/// ベクトルの成分を取得する
/// </summary>
/// <param name="vector2Start">開始地点のベクトル</param>
/// <param name="vector2End">終了地点のベクトル</param>
/// <returns></returns>
Vector2 Component(Vector2 vector2Start, Vector2 vector2End);

/// <summary>
/// ベクトルの内積を取得する
/// </summary>
/// <param name="vector2to1">開始地点のベクトル</param>
/// <param name="vector2to2">終了地点のベクトル</param>
/// <returns></returns>
float DotProduct(Vector2 vector2to1, Vector2 vector2to2);

/// <summary>
/// ベクトルの外積を取得する
/// </summary>
/// <param name="vector2v1">開始地点のベクトル</param>
/// <param name="vector2v2">終了地点のベクトル</param>
/// <returns></returns>
float CrossProduct(Vector2 vector2v1, Vector2 vector2v2);

/// <summary>
/// 開店後の座標を取得する
/// </summary>
/// <param name="pos">回転させたい座標</param>
/// <param name="centerPos">回転の中心となる座標</param>
/// <param name="theta">回転時のtheta</param>
/// <returns></returns>
Vector2 Rotate(Vector2 pos, Vector2 centerPos, float theta);

/// <summary>
/// 開店後の座標を取得する(float)
/// </summary>
/// <param name="pos">回転させたい座標</param>
/// <param name="centerPos">回転の中心となる座標</param>
/// <param name="theta">回転時のtheta</param>
/// <returns></returns>
float Rotate(float pos, float centerPos, float theta);

/// <summary>
/// ベクトルからラジアンに変換する
/// </summary>
/// <param name="vector2">変換したいベクトル</param>
/// <returns></returns>
float VectorToRadian(Vector2 vector2v1);

/// <summary>
/// ラジアンからベクトル変換する
/// </summary>
/// <param name="radian">変換したいラジアン</param>
/// <returns></returns>
Vector2 RadianToVector(float radian);

/// <summary>
/// ベクトルからディグリーに変換する
/// </summary>
/// <param name="vector">変換したいベクトル</param>
/// <returns></returns>
float VectorToDegree(Vector2 vector);

/// <summary>
/// ディグリーからベクトル変換する
/// </summary>
/// <param name="degree"変換したいディグリー</param>
/// <returns></returns>
Vector2 DegreeToVector(float degree);
