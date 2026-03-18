#pragma once
#include <cmath>

struct Vector2 { //vector to represent a 2 dimensional quantity, hence the name Vector2
    double x;
    double y;
    //switch to initializer lists later for compactness, but its ok for now logically its the same thing
    Vector2() { 
        x = 0.0;
        y = 0.0;
    }
    Vector2(double xVal, double yVal) {
        x = xVal;
        y = yVal;
    }

    //basic math functions
    Vector2 add(const Vector2& other) const { //function to add two vectors. const makes sure function doesn't modify the current object
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 subtract(const Vector2& other) const { //function to subtract two vectors
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 multiply(double scalar) const { //scalar multiplication function
        return Vector2(x * scalar, y * scalar);
    }

    double magnitude() const { //magnitude of vector function
        return std::sqrt(x * x + y * y);
    }

    Vector2 normalise() const {
        double mag = magnitude();
        if(mag < 1e-8) { //comparing floats can be problematic (cs159 knowledge omg)
            return Vector2(0.0, 0.0);
        }
        return Vector2(x / mag, y / mag);
    }

    double dot(const Vector2& other) const { //dot product function
        return ((x * other.x) + (y * other.y));
    }
};