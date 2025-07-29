#ifndef resource_guard_hpp
#define resource_guard_hpp

#include <functional>
#include <memory>

/**
 * @brief RAII resource guard for automatic cleanup
 * 
 * This class provides automatic resource cleanup using RAII principles.
 * The cleanup function is called when the guard goes out of scope.
 */
class ResourceGuard {
public:
    using CleanupFunction = std::function<void()>;
    
    /**
     * @brief Construct a resource guard with a cleanup function
     * @param cleanup Function to call during cleanup
     */
    explicit ResourceGuard(CleanupFunction cleanup) 
        : cleanup_(std::move(cleanup)), active_(true) {}
    
    /**
     * @brief Destructor - automatically calls cleanup function
     */
    ~ResourceGuard() {
        if (active_ && cleanup_) {
            try {
                cleanup_();
            } catch (...) {
                // Ignore exceptions during cleanup to prevent termination
            }
        }
    }
    
    /**
     * @brief Prevent copy construction
     */
    ResourceGuard(const ResourceGuard&) = delete;
    
    /**
     * @brief Prevent copy assignment
     */
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    
    /**
     * @brief Allow move construction
     */
    ResourceGuard(ResourceGuard&& other) noexcept 
        : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
        other.active_ = false;
    }
    
    /**
     * @brief Move assignment operator
     * @param other The resource guard to move from
     * @return Reference to this resource guard
     */
    ResourceGuard& operator=(ResourceGuard&& other) noexcept {
        if (this != &other) {
            // Clean up current resources if active
            if (active_ && cleanup_) {
                try {
                    cleanup_();
                } catch (...) {
                    // Ignore exceptions during cleanup
                }
            }
            
            // Move resources from other
            cleanup_ = std::move(other.cleanup_);
            active_ = other.active_;
            
            // Reset other
            other.active_ = false;
            other.cleanup_ = nullptr;
        }
        return *this;
    }
    
    /**
     * @brief Manually release the resource (cleanup is called immediately)
     */
    void release() {
        if (active_ && cleanup_) {
            cleanup_();
            active_ = false;
        }
    }
    
    /**
     * @brief Check if the guard is still active
     */
    bool isActive() const { return active_; }

private:
    CleanupFunction cleanup_;
    bool active_;
};

/**
 * @brief Helper function to create a resource guard
 * @param cleanup Function to call during cleanup
 * @return ResourceGuard instance
 */
inline ResourceGuard makeResourceGuard(ResourceGuard::CleanupFunction cleanup) {
    return ResourceGuard(std::move(cleanup));
}

#endif /* resource_guard_hpp */ 