#ifndef VOLUME4D_H
#define VOLUME4D_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <cstddef>

class Volume4D {
private:
    std::vector<std::vector<std::vector<std::vector<float>>>> data;
    std::size_t dim_x, dim_y, dim_z, dim_t;

public:
    // Constructors
    Volume4D();
    Volume4D(std::size_t x, std::size_t y, std::size_t z, std::size_t t);
    
    // Destructor
    ~Volume4D();
    
    // Copy constructor and assignment operator
    Volume4D(const Volume4D& other);
    Volume4D& operator=(const Volume4D& other);
    
    // Move constructor and assignment operator
    Volume4D(Volume4D&& other) noexcept;
    Volume4D& operator=(Volume4D&& other) noexcept;
    
    // Access methods
    float& at(std::size_t x, std::size_t y, std::size_t z, std::size_t t);
    const float& at(std::size_t x, std::size_t y, std::size_t z, std::size_t t) const;
    
    // Size and capacity
    std::size_t size_x() const { return dim_x; }
    std::size_t size_y() const { return dim_y; }
    std::size_t size_z() const { return dim_z; }
    std::size_t size_t() const { return dim_t; }
    std::size_t total_elements() const;
    bool empty() const;
    
    // Resize and clear
    void resize(std::size_t x, std::size_t y, std::size_t z, std::size_t t);
    void clear();
    
    // Fill methods
    void fill(float value);
    void fill_random(float min_val = 0.0f, float max_val = 1.0f);
    
    // Display methods
    void print_info() const;
    void print_slice(std::size_t time, std::size_t slice) const;
};

#endif // VOLUME4D_H 