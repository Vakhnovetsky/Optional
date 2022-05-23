#include <algorithm>
#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value)
        : value_(new (&data_[0]) T(value))
        , is_initialized_(true) {
    }

    Optional(T&& value)
        : value_(new (&data_[0]) T(std::move(value)))
        , is_initialized_(true) {
    }

    Optional(const Optional& other) {
        if (other.HasValue()) {
            value_ = new (&data_[0]) T(*other);
            is_initialized_ = other.is_initialized_;
        }
    }

    Optional(Optional&& other) noexcept {
        if (other.HasValue()) {
            value_ = new (&data_[0]) T(std::move(*other));
            is_initialized_ = other.is_initialized_;
        }
    }

    Optional& operator=(const T& value) {
        if (HasValue()) {
            *value_ = value;
        }
        else {
            is_initialized_ = true;
            value_ = new (&data_[0]) T(value);
        }
        return *this;
    }

    Optional& operator=(T&& rhs) {
        if (HasValue()) {
            *value_ = std::move(rhs);
        }
        else {
            is_initialized_ = true;
            value_ = new (&data_[0]) T(std::move(rhs));
        }
        return *this;
    }

    Optional& operator=(const Optional& rhs) {
        //empty - non empty
        if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(*rhs);
            is_initialized_ = true;
        }

        //non empty - empty
        else if (HasValue() && !rhs.HasValue()) {
            value_->~T();
            is_initialized_ = false;
        }

        //non empty - non empty
        else if (HasValue() && rhs.HasValue() && this != &rhs) {
            //*value_ = rhs.Value();
            *value_ = *rhs.value_;
        }
        return *this;
    }

    Optional& operator=(Optional&& rhs) noexcept {
        //empty - non empty
        if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(std::move(*rhs));
            is_initialized_ = true;
        }

        //non empty - empty
        else if (HasValue() && !rhs.HasValue()) {
            value_->~T();
            is_initialized_ = false;
        }

        //non empty - non empty
        else if (HasValue() && rhs.HasValue() && this != &rhs) {
            //*value_ = std::move(rhs.Value());
            *value_ = std::move(*rhs.value_);
        }
        return *this;
    }

    ~Optional() {
        if (HasValue()) {
            value_->~T();
        }
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*()& {
        return *value_;
    }

    [[nodiscard]] T operator*()&& {
        return std::move(*value_);
    }

    const T& operator*() const& {
        return *value_;
    }

    T* operator->() {
        return value_;
    }

    const T* operator->() const {
        return value_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value()& {
        if (!HasValue()) throw BadOptionalAccess();
        return *value_;
    }

    [[nodiscard]] T Value()&& {
        if (!HasValue()) throw BadOptionalAccess();
        return std::move(*value_);
    }

    const T& Value() const& {
        if (!HasValue()) throw BadOptionalAccess();
        return *value_;
    }

    void Reset() {
        if (HasValue()) {
            value_->~T();
            is_initialized_ = false;
        }
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        Reset();
        value_ = new (&data_[0]) T(std::forward<Args>(args)...);
        is_initialized_ = true;
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    T* value_ = nullptr;
    bool is_initialized_ = false;
};