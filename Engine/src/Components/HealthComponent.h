#pragma once

struct HealthComponent
{
    int health_percentage;
    HealthComponent(int health_percentage = 0) {
        this->health_percentage = health_percentage;
    }
};