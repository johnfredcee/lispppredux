#include <iostream>
#include <memory>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <string>
#include <variant>
#include <cassert>

/**************************** MODEL ******************************/

using fixnum = long;
using character = char;
using str = std::string;

constexpr static unsigned FIXNUM = 0;
constexpr static unsigned BOOLEAN = 1;
constexpr static unsigned CHARACTER = 2;
constexpr static unsigned STRING = 3;
constexpr static unsigned CONS = 4;

class lispobj;

struct conscell
{
    std::shared_ptr<lispobj> car;
    std::shared_ptr<lispobj> cdr;
};

class lispobj
{
  public:
    lispobj() : tag(FIXNUM)
    {
        intvalue = 0;
    }

    explicit lispobj(fixnum v) : tag(FIXNUM), intvalue(v)
    {
    }

    explicit lispobj(bool v) : tag(BOOLEAN), boolvalue(v)
    {
    }

    explicit lispobj(character v) : tag(CHARACTER), chrvalue(v)
    {
    }

    explicit lispobj(str v) : tag(STRING)
    {
        Init(strvalue, v);
    }

    explicit lispobj(conscell v) : tag(CONS)
    {
        Init(consvalue, v);
    }

    ~lispobj()
    {
        Destroy();
    }

    lispobj(const lispobj &o)
    {
        Copy(o);
    }

    lispobj &operator=(const lispobj &o)
    {
        if (&o != this)
        {
            Destroy();
            Copy(o);
        }
    }

    template <typename T>
    const T get()
    {
    }

    template <>
    const fixnum get<fixnum>()
    {
        return intvalue;
    }

    template <>
    const bool get<bool>()
    {
        return boolvalue;
    }

    template <>
    const character get<character>()
    {
        return chrvalue;
    }

    template <>
    const str get<str>()
    {
        return strvalue;
    }

    template <>
    const conscell get<conscell>()
    {
        return consvalue;
    }

    void set_car(std::shared_ptr<lispobj> in_car)
    {
        assert(tag == CONS);
        consvalue.car = in_car;
    }

    void set_cdr(std::shared_ptr<lispobj> in_cdr)
    {
        assert(tag == CONS);
        consvalue.cdr = in_cdr;
    }

    uint32_t index()
    {
        return tag;
    }

  private:
    template <typename T>
    void Init(T &member, const T &val)
    {
        new (&member) T(val);
    }

    void Destroy()
    {
        if (tag == STRING)
        {
            strvalue.~str();
        }
        if (tag == CONS)
        {
            consvalue.~conscell();
        }
    }

    void Copy(const lispobj &o)
    {
        assert(tag == o.tag);
        switch (o.tag)
        {
        case FIXNUM:
            intvalue = o.intvalue;
            break;
        case BOOLEAN:
            boolvalue = o.boolvalue;
            break;
        case CHARACTER:
            chrvalue = o.chrvalue;
            break;
        case STRING:
            Init(strvalue, o.strvalue);
            break;
        case CONS:
            Init(consvalue, o.consvalue);
            break;
        }
        tag = o.tag;
    }
    uint32_t tag;

    union {
        /* data */
        fixnum intvalue;
        bool boolvalue;
        character chrvalue;
        str strvalue;
        conscell consvalue;
    };
};

using obj = lispobj;
using objptr = std::shared_ptr<lispobj>;

template <typename T>
T get(lispobj &o)
{
    return o.get<T>();
};

objptr gnil;
objptr gfalse;
objptr gtrue;

objptr alloc_object(void)
{
    return std::make_shared<obj>();
}

template <typename T>
objptr make_object(T arg)
{
    return std::make_shared<obj>(arg);
}

template <unsigned I>
inline bool is_type(objptr o)
{
    return o->index() == I;
}

objptr cons(objptr in_car, objptr in_cdr)
{
    return make_object<conscell>(conscell{in_car, in_cdr});
}

objptr car(objptr in_obj)
{
    return in_obj->get<conscell>().car;
}

void set_car(objptr in_obj, objptr in_value)
{
    in_obj->set_cdr(in_value);
}

objptr cdr(objptr in_obj)
{
    return in_obj->get<conscell>().cdr;
}

void set_cdr(objptr in_obj, objptr in_value)
{
    in_obj->set_cdr(in_value);
}

#define caar(obj) car(car(obj));
#define cadr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
#define caaar(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))

void init(void)
{
    gfalse = make_object<bool>(false);
    gtrue = make_object<bool>(true);
    gnil = std::shared_ptr<lispobj>();
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

objptr read(std::istream &in);

objptr read_pair(std::istream &in)
{
    char c;
    objptr car_obj;
    objptr cdr_obj;

    eat_whitespace(in);
    if (in.get(c))
    {
        if (c == ')')
        {
            return gnil;
        }
        in.unget();
        car_obj = read(in);
        eat_whitespace(in);
        if (in.get(c))
        {
            if (c == '.')
            {
                // read improper iist
                c = in.peek();
                if (!isspace(c))
                {
                    std::cerr << "Dot not followed by whitespace" << std::endl;
                    exit(1);
                }
                cdr_obj = read(in);
                eat_whitespace(in);
                in.get(c);
                if (c != ')')
                {
                    std::cerr << "Missing close paren" << std::endl;
                    exit(1);
                }
                return cons(car_obj, cdr_obj);
            }
            else
            {
                // read list
                in.unget();
                cdr_obj = read_pair(in);
                return cons(car_obj, cdr_obj);
            }
        }
        else
        {
            std::cerr << "Premature end of file." << std::endl;
            exit(1);
        }
    }
    else
    {
        std::cerr << "Premature end of file." << std::endl;
        exit(1);
    }
    return gnil;
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
    else if (c == '"')
    {
        std::string tempstring;
        tempstring.reserve(1024);
        while (in.get(c))
        {
            if (c == '"')
                break;
            if (c == '\\')
            {
                if (in.get(c))
                {
                    if (c == 'n')
                        c = '\n';
                    else if (c == 'r')
                        c = '\r';
                    else if (c == 't')
                        c = '\t';
                }
                else
                {
                    std::cerr << "Non terminated string litteral" << std::endl;
                    exit(1);
                }
            }
            tempstring.push_back(c);
        }
        return make_object<str>(tempstring);
    }
    else if (c == '(')
    {
        return read_pair(in);
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

void write(objptr obj);

void write_pair(objptr obj)
{
    objptr car_obj;
    objptr cdr_obj;

    car_obj = car(obj);
    cdr_obj = cdr(obj);
    write(car_obj);
    if (!cdr_obj)
    {
        return;
    }
    if (is_type<CONS>(cdr_obj))
    {
        std::cout << " ";
        write(cdr_obj);
    }
    else
    {
        std::cout << " . ";
        write(cdr_obj);
    }
}

void write(objptr obj)
{
    char c;
    str s;
    if (!obj)
    {
        std::cout << "nil";
    }
    else
    {
        switch (obj->index())
        {
        case FIXNUM:
            std::cout << get<fixnum>(*obj);
            break;
        case BOOLEAN:
            std::cout << (get<bool>(*obj) ? "#t" : "#f");
            break;
        case CHARACTER:
            c = get<character>(*obj);
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
        case STRING:
            s = get<str>(*obj);
            std::cout << ""
                         "";
            for (auto c : s)
            {
                switch (c)
                {
                case '\n':
                    std::cout << "\\n";
                    break;
                case '\r':
                    std::cout << "\\r";
                    break;
                case '"':
                    std::cout << '"';
                    break;
                default:
                    std::cout << c;
                    break;
                }
            }
            std::cout << ""
                         "";
            break;
        case CONS:
            std::cout << "(";
            write_pair(obj);
            std::cout << ")";
        default:
            std::cerr << "Cannot write unknown type" << std::endl;
            exit(1);
        }
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
