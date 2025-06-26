#pragma once

#include <version>

// Check if std::expected is available
#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
    #include <expected>
#else
    // Minimal std::expected implementation for compatibility
    #include <variant>
    #include <utility>
    
    namespace std {
        template<typename T, typename E>
        class expected {
        public:
            using value_type = T;
            using error_type = E;
            
            // Constructors
            expected() requires std::is_default_constructible_v<T> : data_(T{}) {}
            expected(const T& value) : data_(value) {}
            expected(T&& value) : data_(std::move(value)) {}
            
            template<typename... Args>
            expected(std::in_place_t, Args&&... args) : data_(std::in_place_type<T>, std::forward<Args>(args)...) {}
            
            expected(unexpected<E> e) : data_(std::move(e.error())) {}
            
            // Observers
            constexpr bool has_value() const noexcept {
                return std::holds_alternative<T>(data_);
            }
            
            constexpr explicit operator bool() const noexcept {
                return has_value();
            }
            
            constexpr const T& value() const& {
                return std::get<T>(data_);
            }
            
            constexpr T& value() & {
                return std::get<T>(data_);
            }
            
            constexpr T&& value() && {
                return std::get<T>(std::move(data_));
            }
            
            constexpr const E& error() const& {
                return std::get<E>(data_);
            }
            
            constexpr E& error() & {
                return std::get<E>(data_);
            }
            
        private:
            std::variant<T, E> data_;
        };
        
        // Specialization for void
        template<typename E>
        class expected<void, E> {
        public:
            using value_type = void;
            using error_type = E;
            
            expected() : data_(std::monostate{}) {}
            expected(unexpected<E> e) : data_(std::move(e.error())) {}
            
            constexpr bool has_value() const noexcept {
                return std::holds_alternative<std::monostate>(data_);
            }
            
            constexpr explicit operator bool() const noexcept {
                return has_value();
            }
            
            constexpr const E& error() const& {
                return std::get<E>(data_);
            }
            
            constexpr E& error() & {
                return std::get<E>(data_);
            }
            
        private:
            std::variant<std::monostate, E> data_;
        };
        
        template<typename E>
        class unexpected {
        public:
            unexpected(E e) : error_(std::move(e)) {}
            
            const E& error() const& { return error_; }
            E& error() & { return error_; }
            E&& error() && { return std::move(error_); }
            
        private:
            E error_;
        };
        
        template<typename E>
        unexpected(E) -> unexpected<E>;
    }
#endif