#include "json.h"

using namespace std;

namespace json {

static const double EPS = 1e-9;

namespace {

Node LoadNode(istream& input);

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node ProcessNum(istream& input) {
    Number num = LoadNumber(input);
    size_t index = num.index();
    if (index == 0) {
        return Node(get<int>(num));
    } else {
        return Node(get<double>(num));
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadArray(istream& input) {
    Array result;
    bool closed_found = false;

    for (char c; input >> c;) {
        if (c == ']') {
            closed_found = true;
            break;
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!closed_found) {
        throw ParsingError("There is no ']' in input stream");
    }

    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    bool closed_found = false;

    for (char c; input >> c;) {
        if (c == '}') {
            closed_found = true;
            break;
        }
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (!closed_found) {
        throw ParsingError("There is no '}' in input stream");
    }

    return Node(move(result));
}

string ReadWord(istream& input) {
    std::string res;
    while (std::isalpha(input.peek())) {
        res.push_back(static_cast<char>(input.get()));
    }
    return res;
}

Node LoadNull(istream& input) {
    string s = ReadWord(input);
    if (s != "null"s) {
        throw ParsingError("Error null reading");
    }
    return Node(nullptr);
}

Node LoadBool(istream& input, char c) {
    string s;
    if (c == 't') {
        s = ReadWord(input);
        if (s != "true"s) {
            throw ParsingError("Error bool true reading");
        }
        return Node(true);
    } else if (c == 'f') {
        s = ReadWord(input);
        if (s != "false"s) {
            throw ParsingError("Error bool false reading");
        }
        return Node(false);
    }
    throw;
}

Node LoadNode(istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input, c);
    } else {
        input.putback(c);
        return ProcessNum(input);
        
    }
}

}  // namespace


// AsSomething
int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("error: node value type is not int");
    }
    return get<int>(*this);
}
double Node::AsDouble() const {
    if (IsInt()) {
        return get<int>(*this);
    } else if (IsPureDouble()) {
        return get<double>(*this);
    } else {
        throw logic_error("error: node value type is not double");
    }
}
const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("error: node value type is not string");
    }
    return get<string>(*this);
}
bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("error: node value type is not bool");
    }
    return get<bool>(*this);
}
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("error: node value type is not array");
    }
    return get<Array>(*this);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("error: node value type is not map");
    }
    return get<Dict>(*this);
}
// ~AsSomething


// IsSomething
bool Node::IsNull() const {
    return std::holds_alternative<nullptr_t>(*this);
};
bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
};
bool Node::IsDouble() const {
    return std::holds_alternative<int>(*this)
         || std::holds_alternative<double>(*this);
};
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
};
bool Node::IsString() const {
    return std::holds_alternative<string>(*this);
};
bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
};
bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
};
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
};
// ~IsSomething

bool Node::operator<(const Node& other) const {
    if (IsString() && other.IsString()) {
        return AsString() < other.AsString();
    } else if (IsInt() && other.IsInt()) {
        return AsInt() < other.AsInt();
    } else if (IsDouble() && other.IsDouble()) {
        return AsDouble() < other.AsDouble();
    } else {
        return false;
    }
}

// Document
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}
// ~Document

// Print
struct PrintContext {
    PrintContext() = default;
    PrintContext(int value) 
        : indent(value) {
    }
    PrintContext(int value, int step) 
        : indent(value), indent_step(step) {
    }

    PrintContext& Increase(int times = 1) {
        indent += indent_step * times;
        return *this;
    }
    PrintContext& Reduce(int times = 1) {
        indent -= indent_step * times;
        return *this;
    }
    int indent = 0;
    int indent_step = 4;
};

std::ostream& operator<<(std::ostream& out, const PrintContext& context) {
    for (int i = 0; i < context.indent; ++i) {
        out.put(' ');
    }
    return out;
}

void PrintNode(ostream& out, const Node& node, PrintContext& context);

void PrintArray(ostream& out, const Array& arr, PrintContext& context) {
    out << context << "[\n"sv;
    context.Increase();
    bool is_first = true;
    for (const auto& node : arr) {
        if (!is_first) {
            out << ",\n";
        }
        is_first = false;
        PrintNode(out, node, context);
    }
    out << '\n' << context.Reduce() << "]"sv;
}

void PrintDict(ostream& out, const Dict& dict, PrintContext& context) {
    out << context << "{\n"sv;
    context.Increase();
    bool is_first = true;
    for (const auto& [key_str, node] : dict) {
        if (!is_first) {
            out <<",\n"sv;
        }
        is_first = false;
        out << context << "\""sv << key_str << "\" : "sv;
        PrintNode(out, node, context);
    }
    out << '\n' << context.Reduce() << "}"sv;
}

void PrintString(ostream& out, const string& str) {
    out << "\""s;
    for (const auto& c : str) {
        if (c == '\"') {
            out << "\\\""s;
        } else if (c == '\n') {
            out << "\\n"s;
        } else if (c == '\r') {
            out << "\\r"s;
        } else if (c == '\t') {
            out << "\\t"s;
        } else if (c == '\\') {
            out << "\\\\"s;
        } else {
            out << c;
        }
    }
    out << "\""s;
}
struct GetVal {
    ostream& out;
    PrintContext& context;

    void operator()(std::nullptr_t) const {
        out << "null";
    }
    void operator()(int value) const {
        out << value;
    }
    void operator()(double value) const {
        out << value;
    }
    void operator()(std::string value) const {
        PrintString(out, value);
    }
    void operator()(bool value) const {
        out << (value ? "true" : "false");
    }
    void operator()(json::Dict dict) const {
        PrintDict(out, dict, context);
    }
    void operator()(json::Array arr) const {
        PrintArray(out, arr, context);
    }
};

void PrintNode(ostream& out, const Node& node, PrintContext& context) {
    std::visit(GetVal{out, context}, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext context;
    PrintNode(output, doc.GetRoot(), context);
}
// ~Print

}  // namespace json