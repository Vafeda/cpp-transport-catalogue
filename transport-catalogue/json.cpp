#include "json.h"
#include <cmath>

using namespace std;

namespace json {

    namespace {

        Node LoadNull(istream& input) {
            char c;
            std::string n;
            for (int i = 0; isalpha(input.peek()); i++) {
                input >> c;
                n += c;
            };

            if (n == "null") {
                return Node{};
            }
            else {
                throw ParsingError("Error initializing Node to null"s);
            }
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

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

            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

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
                    try {
                        return Node{ std::stoi(parsed_num) };
                    }
                    catch (...) {

                    }
                }
                return Node{ std::stod(parsed_num) };
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadBool(istream& input, char c) {
            std::string n;
            n += c;

            if (c == 't') {
                for (int i = 0; isalpha(input.peek()); ++i) {
                    input >> c;
                    n += c;
                }
                if (n == "true") {
                    return Node{ true };
                }
                else {
                    throw ParsingError("Error initializing Node to true"s);
                }
            }
            else {
                for (int i = 0; isalpha(input.peek()); ++i) {
                    input >> c;
                    n += c;
                }
                if (n == "false") {
                    return Node{ false };
                }
                else {
                    throw ParsingError("Error initializing Node to false"s);
                }
            }

            return Node{};
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
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
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node{ s };
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            char c;
            while (true) {
                if (!(input >> c)) {
                    throw ParsingError("The array is not finished. The \"]\" sign is missing"s);
                }

                if (c == ']') break;

                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;

            char c;
            while (true) {
                if (!(input >> c)) {
                    throw ParsingError("The array is not finished. The \"}\" sign is missing"s);
                }

                if (c == '}') break;

                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                return LoadBool(input, c);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (isdigit(c) || (c == '-' && isdigit(input.peek()))) {
                input.putback(c);
                return LoadNumber(input);
            }
            throw ParsingError("");
        }

    }

    //----------------------------------------------
    //---------------Type_checking------------------
    //----------------------------------------------
    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    //----------------------------------------------
    //----------------Return_value------------------
    //----------------------------------------------
    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("The value isn't integer");
        }

        return std::get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("The value isn't bool");
        }

        return std::get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        throw std::logic_error("The value isn't double");
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("The value isn't string");
        }

        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("The value isn't array");
        }

        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("The value isn't dictionary");
        }

        return std::get<Dict>(*this);
    }


    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintContext ctx{ output, 4, 0 };
        visit(PrintNode{ ctx }, doc.GetRoot().GetVariant());
    }

}