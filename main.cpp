#include <iostream>
#include <variant>
#include <memory>

/**************************** MODEL ******************************/

using fixnum = long;

constexpr static unsigned FIXNUM = 0;
constexpr static unsigned BOOLEAN = 1;

using obj = std::variant<fixnum, bool>;

using objptr = std::shared_ptr<obj>;

objptr gfalse;
objptr gtrue;

objptr alloc_object(void)
{
    return std::make_shared<obj>();
}

template <typename T>
objptr make_object(T value)
{
    return std::make_shared<obj>(value);
}

template <unsigned I>
inline bool is_type(objptr o)
{
    return o->index() == I;
}

void init(void)
{
    gfalse = make_object<bool>(false);
    gtrue = make_object<bool>(true);
}

/***************************** READ ******************************/

inline bool is_delimiter(char c)
{
    constexpr char delims[5] = {'(', ')', '"', ';'};
    return (isspace(c) || (std::find(std::begin(delims), std::end(delims), c) != std::end(delims)));
}

inline void eat_whitespace(std::istream &in)
{
    char c;

    while (in.get(c))
    {
        if (isspace(c))
            continue;
        else if (c == ';')
        {
            while (in.get(c))
            {
                if (c != '\n')
                    continue;
            }
        }
        in.unget();
        break;
    }
}

objptr read(std::istream &in)
{
    char c;
    short sign = 1;
    fixnum num = 0;

    eat_whitespace(in);

    if (!in.get(c))
    {
        std::cerr << "End of input" << std::endl;
        exit(1);
    }

    if (c == '#')
    { /* read a boolean */
        if (!in.get(c))
        {
            std::cerr << "hash before eof" << std::endl;
            exit(1);
        }
        switch (c)
        {
        case 't':
            return gtrue;
        case 'f':
            return gfalse;
        default:
            std::cerr << "unknown boolean literal" << std::endl;
            exit(1);
        }
    }
    else if ((isdigit(c) || (c == '-') && isdigit(in.peek())))
    {
        /* first, check the sign */
        if (c == '-')
        {
            sign = -1;
        }
        else
        {
            in.unget();
        }
        /* loop over the number and accumulate the value */
        while (in.get(c))
        {
            if (isdigit(c))
            {
                num = num * 10 + (c - '0');
            }
            else
            {
                break;
            }
        }
        /* give it the sign */
        num *= sign;
        if (is_delimiter(c))
        {
            in.unget();
            return make_object<fixnum>(num);
        }
        else
        {
            std::cerr << "Nubmer not followed by delimiter " << std::endl;
            exit(1);
        }
    }
    else
    {
        std::cerr << "Bad input. Unexpected '" << c << "'" << std::endl;
        exit(1);
    }
    std::cerr << "Bad input. Unexpected '" << c << "'" << std::endl;
    exit(1);
}

/*************************** EVALUATE ****************************/

/* until we have lists and symbols just echo */
objptr eval(objptr exp)
{
    return exp;
}

/**************************** PRINT ******************************/

void write(objptr obj)
{
    switch (obj->index())
    {
    case FIXNUM:
        std::cout << std::get<fixnum>(*obj) << std::endl;
        break;
    case BOOLEAN:
        std::cout << (std::get<bool>(*obj) ? "#t" : "#f") << std::endl;
        break;
    default:
        std::cerr << "Cannot write unknown type" << std::endl;
        exit(1);
    }
}

/***************************** REPL ******************************/

int main(int argc, const char **argv)
{
    std::cout << "Welcome to Bootstrap Scheme." << std::endl;
    std::cout << "Use ctrl-c to exit." << std::endl;

    init();

    while (true)
    {
        std::cout << ">";
        write(eval(read(std::cin)));
        std::cout << std::endl;
    }
    return 0;
}
