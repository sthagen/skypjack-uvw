#ifndef UVW_UV_TYPE_INCLUDE_H
#define UVW_UV_TYPE_INCLUDE_H

#include <memory>
#include <type_traits>
#include <utility>
#include "config.h"
#include "loop.h"

namespace uvw {

/**
 * @brief Wrapper class for underlying types.
 *
 * It acts mainly as a wrapper around data structures of the underlying library.
 */
template<typename U>
struct uv_type {
    explicit uv_type(loop::token, std::shared_ptr<loop> ref) noexcept
        : owner{std::move(ref)} {}

    uv_type(const uv_type &) = delete;
    uv_type(uv_type &&) = delete;

    uv_type &operator=(const uv_type &) = delete;
    uv_type &operator=(uv_type &&) = delete;

    /**
     * @brief Gets the loop from which the resource was originated.
     * @return A reference to a loop instance.
     */
    [[nodiscard]] loop &parent() const noexcept {
        return *owner;
    }

    /**
     * @brief Gets the underlying raw data structure.
     *
     * This function should not be used, unless you know exactly what you are
     * doing and what are the risks.<br/>
     * Going raw is dangerous, mainly because the lifetime management of a loop,
     * a handle or a request is in charge to the library itself and users should
     * not work around it.
     *
     * @warning
     * Use this function at your own risk, but do not expect any support in case
     * of bugs.
     *
     * @return The underlying raw data structure.
     */
    [[nodiscard]] const U *raw() const noexcept {
        return &resource;
    }

    /**
     * @brief Gets the underlying raw data structure.
     *
     * This function should not be used, unless you know exactly what you are
     * doing and what are the risks.<br/>
     * Going raw is dangerous, mainly because the lifetime management of a loop,
     * a handle or a request is in charge to the library itself and users should
     * not work around it.
     *
     * @warning
     * Use this function at your own risk, but do not expect any support in case
     * of bugs.
     *
     * @return The underlying raw data structure.
     */
    [[nodiscard]] U *raw() noexcept {
        return &resource;
    }

protected:
    ~uv_type() = default;

private:
    std::shared_ptr<loop> owner;
    U resource{};
};

} // namespace uvw

#endif // UVW_UV_TYPE_INCLUDE_H
