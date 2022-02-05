#pragma once


namespace mandalang {


    enum class type_tag {
        floating_point, integer, boolean, composite
    }; // type_tag


    struct composite_type;
    struct type {
        type_tag tag;
        composite_type* composite{nullptr};

        bool operator == (type const& other) const noexcept;
        bool operator != (type const& other) const noexcept;
    };


    enum class composite_type_tag {
        function, vector
    };


    struct composite_type {

        static constexpr auto max_function_parameters = 16;

        composite_type_tag tag;

        union {
            struct function {
                type result;
                unsigned arity;
                type parameters[max_function_parameters];

                bool operator == (function const& other) const noexcept {
                    if(arity != other.arity)
                        return false;
                    if(result != other.result)
                        return false;
                    for(auto i = 0u; i != arity; ++i)
                        if(parameters[i] != other.parameters[i])
                            return false;
                    return true;
                }


                bool operator != (function const& other) const noexcept {
                    return !(*this  == other);
                }
            } function;
            type item;
        };

        composite_type() noexcept { }
        composite_type(composite_type const&) noexcept = default;
        composite_type& operator = (composite_type const&) noexcept = default;

        composite_type(type result, unsigned arity, type parameters[]) noexcept:
                tag{composite_type_tag::function}, function{result, arity} {
            for(auto i = 0u; i != arity; ++i)
                function.parameters[i] = parameters[i];
        }

        composite_type(composite_type_tag tag, type item) noexcept:
            tag{tag}, item{item}
        { }

        bool operator == (composite_type const& other) const noexcept {
            if(tag != other.tag)
                return false;
            switch(tag) {
                case composite_type_tag::function:
                    return function == other.function;
                case composite_type_tag::vector:
                    return item == other.item;
                default:
                    return true;
            }
        }


        bool operator != (composite_type const& other) const noexcept {
            return !(*this == other);
        }
    }; // type


    inline bool type::operator == (type const& other) const noexcept {
        return tag == type_tag::composite ? *composite == *other.composite : tag == other.tag;
    }


    inline bool type::operator != (type const& other) const noexcept {
        return !(*this == other);
    }


    template<typename S> S& operator << (S& stream, type const& type);

    template<typename S> S& operator << (S& stream, composite_type const& composite_type) {
        switch(composite_type.tag) {
            case composite_type_tag::function:
                stream << "fn (";
                if(composite_type.function.arity != 0) {
                    stream << composite_type.function.parameters[0];
                    for(auto i = 1u; i != composite_type.function.arity; ++i)
                        stream << ", " << composite_type.function.parameters[i];
                }
                stream << ") -> " << composite_type.function.result;
                return stream;
            case composite_type_tag::vector:
                stream << "vector[" << composite_type.item << ']';
            default:
                return stream << "unknown";
        }
    }


    template<typename S> S& operator << (S& stream, type const& type) {
        switch(type.tag) {
            case type_tag::floating_point:
                return stream << "double";
            case type_tag::integer:
                return stream << "integer";
            case type_tag::boolean:
                return stream << "boolean";
            case type_tag::composite:
                return stream << *type.composite;
            default:
                return stream << "unknown";
        }
    }

} // namespace mandalang
