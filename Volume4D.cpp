#include "Volume4D.h"
#include <random>
#include <algorithm>

// Default constructor
Volume4D::Volume4D() : dim_x(0), dim_y(0), dim_z(0), dim_t(0) {}

// Parameterized constructor
Volume4D::Volume4D(std::size_t x, std::size_t y, std::size_t z, std::size_t t) 
    : dim_x(x), dim_y(y), dim_z(z), dim_t(t) {
    resize(x, y, z, t);
}

// Destructor
Volume4D::~Volume4D() = default;

// Copy constructor
Volume4D::Volume4D(const Volume4D& other) 
    : data(other.data), dim_x(other.dim_x), dim_y(other.dim_y), dim_z(other.dim_z), dim_t(other.dim_t) {}

// Copy assignment operator
Volume4D& Volume4D::operator=(const Volume4D& other) {
    if (this != &other) {
        data = other.data;
        dim_x = other.dim_x;
        dim_y = other.dim_y;
        dim_z = other.dim_z;
        dim_t = other.dim_t;
    }
    return *this;
}

// Move constructor
Volume4D::Volume4D(Volume4D&& other) noexcept 
    : data(std::move(other.data)), dim_x(other.dim_x), dim_y(other.dim_y), dim_z(other.dim_z), dim_t(other.dim_t) {
    other.dim_x = other.dim_y = other.dim_z = other.dim_t = 0;
}

// Move assignment operator
Volume4D& Volume4D::operator=(Volume4D&& other) noexcept {
    if (this != &other) {
        data = std::move(other.data);
        dim_x = other.dim_x;
        dim_y = other.dim_y;
        dim_z = other.dim_z;
        dim_t = other.dim_t;
        other.dim_x = other.dim_y = other.dim_z = other.dim_t = 0;
    }
    return *this;
}

// Access methods
float& Volume4D::at(std::size_t x, std::size_t y, std::size_t z, std::size_t t) {
    if (x >= dim_x || y >= dim_y || z >= dim_z || t >= dim_t) {
        throw std::out_of_range("Volume4D::at: Index out of range");
    }
    return data[t][z][y][x];
}

const float& Volume4D::at(std::size_t x, std::size_t y, std::size_t z, std::size_t t) const {
    if (x >= dim_x || y >= dim_y || z >= dim_z || t >= dim_t) {
        throw std::out_of_range("Volume4D::at: Index out of range");
    }
    return data[t][z][y][x];
}

// Size and capacity
std::size_t Volume4D::total_elements() const {
    return dim_x * dim_y * dim_z * dim_t;
}

bool Volume4D::empty() const {
    return data.empty() || dim_x == 0 || dim_y == 0 || dim_z == 0 || dim_t == 0;
}


void Volume4D::clear() {
    data.clear();
    dim_x = dim_y = dim_z = dim_t = 0;
}

void Volume4D::resize(std::size_t x, std::size_t y, std::size_t z, std::size_t t) {
    dim_x = x;
    dim_y = y;
    dim_z = z;
    dim_t = t;
    
    // Resize the 4D vector structure
    data.resize(t);
    for (auto& time_slice : data) {
        time_slice.resize(z);
        for (auto& slice : time_slice) {
            slice.resize(y);
            for (auto& row : slice) {
                row.resize(x, 0.0f);
            }
        }
    }
}

// Fill methods
void Volume4D::fill(float value) {
    for (auto& time_slice : data) {
        for (auto& slice : time_slice) {
            for (auto& row : slice) {
                std::fill(row.begin(), row.end(), value);
            }
        }
    }
}

void Volume4D::fill_random(float min_val, float max_val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min_val, max_val);
    
    for (auto& time_slice : data) {
        for (auto& slice : time_slice) {
            for (auto& row : slice) {
                for (auto& val : row) {
                    val = dis(gen);
                }
            }
        }
    }
}

// Display methods
void Volume4D::print_info() const {
    std::cout << "=== Volume4D Information ===" << std::endl;
    std::cout << "Dimensions: " << dim_x << " × " << dim_y << " × " << dim_z << " × " << dim_t << std::endl;
    std::cout << "Total elements: " << total_elements() << std::endl;
    std::cout << "Memory usage: ~" << std::fixed << std::setprecision(9) 
              << (total_elements() * sizeof(float) / (1024.0 * 1024.0)) << " MB" << std::endl;
    std::cout << "Empty: " << (empty() ? "Yes" : "No") << std::endl;
    std::cout << "===========================" << std::endl;
}

void Volume4D::print_slice(std::size_t time, std::size_t slice) const {
    if (time >= dim_t || slice >= dim_z) {
        std::cout << "Error: Invalid time or slice index" << std::endl;
        return;
    }
    
    std::cout << "Slice at time=" << time << ", slice=" << slice << ":" << std::endl;
    for (std::size_t y = 0; y < dim_y; ++y) {
        for (std::size_t x = 0; x < dim_x; ++x) {
            std::cout << std::setw(6) << std::fixed << std::setprecision(0) 
                      << data[time][slice][y][x] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
} 