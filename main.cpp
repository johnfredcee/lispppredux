#include <iostream>
#include <variant>
#include <memory>

/**************************** MODEL ******************************/

using fixnum = long;
using character = char;

constexpr static unsigned FIXNUM = 0;
constexpr static unsigned BOOLEAN = 1;
constexpr static unsigned CHARACTER = 2;

using obj = std::variant<fixnum, bool, character>;

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
    s
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

inline void eat_expected_string(std::istream &in, const std::string &str)
{
    char c;
    auto str_it = std::begin(str);
    while (str_it != std::end(str))
    {
        if (in.get(c))
        {
            if (c != *str_it)
            {
                std::cerr << "Unexxpected character " << c << " while scanning for " << str.c_str() << std::endl;
                exit(1);
            }
        }
        else
        {
            if (c != *str_it)
            {
                std::cerr << "Unexxpected eof while scanning for " << str.c_str() << std::endl;
                exit(1);
            }
        }
        ++str_it;
    }
}

void peek_expected_delimiter(std::istream &in)
{
    int c = in.peek();
    if (!is_delimiter(c))
    {
        std::cerr << "Missing delimiter " << std::endl;
        exit(1);
    }
}

objptr read_character(std::istream &in)
{
    char c;
    if (in.get(c))
    {
        switch (c)
        {
        case 's':
            if (in.peek() == 'p')
            {
                eat_expected_string(in, "pace");
                peek_expected_delimiter(in);
                return make_object<character>(' ');
            }
            break;
        case 'n':
            if (in.peek() == 'e')
            {
                eat_expected_string(in, "ewline");
                peek_expected_delimiter(in);
                return make_object<character>('\n');
            }
            break;
        }
        peek_expected_delimiter(in);
        return make_object<character>(c);
    }
    else
    {
        std::cerr << "Unexpexted eof in character literal " << std::endl;
        exit(1);
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
        case '\\':
            return read_character(in);
        default:
            std::cerr << "unknown boolean or character literal" << std::endl;
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
    char c;
    switch (obj->index())
    {
    case FIXNUM:
        std::cout << std::get<fixnum>(*obj) << std::endl;
        break;
    case BOOLEAN:
        std::cout << (std::get<bool>(*obj) ? "#t" : "#f") << std::endl;
        break;
    case CHARACTER:
        c = std::get<character>(*obj);
        std::cout << "#\\";
        switch (c)
        {
            case '\n':
                std::cout << "newline";
                break;
            case ' ':
                std::cout << "space";
                break;
            default:
                std::cout << c;
        }
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
