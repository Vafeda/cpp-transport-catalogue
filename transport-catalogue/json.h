#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;


        Node(std::nullptr_t = nullptr) : value_(nullptr) {}
        Node(Array array) : value_(std::move(array)) {}
        Node(Dict dict) : value_(std::move(dict)) {}
        Node(bool b) : value_(b) {}
        Node(int i) : value_(i) {}
        Node(double d) : value_(d) {}
        Node(std::string str) : value_(std::move(str)) {}

        //----------------------------------------------
        //---------------Type_checking------------------
        //----------------------------------------------
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        //----------------------------------------------
        //----------------Return_value------------------
        //----------------------------------------------
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const { return value_; }

        bool operator==(const Node& other) const {
            return value_ == other.value_;
        }

        bool operator!=(const Node& other) const {
            return value_ != other.value_;
        }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() {
            for (int i = 0; i < indent; ++i) {
                out << ' ';
            }
        }

        PrintContext Indented() {
            return { out, indent_step, indent + indent_step };
        }
    };

    struct PrintNode {
    public:
        PrintNode(PrintContext ctx)
            : ctx_(ctx)
        {
        }

        void operator()(nullptr_t) {
            ctx_.out << "null";
        }

        void operator()(int value) {
            ctx_.out << value;
        }

        void operator()(double value) {
            ctx_.out << value;
        }

        void operator()(bool value) {
            ctx_.out << (value ? "true" : "false");
        }

        void operator()(const std::string& str) {
            ctx_.out << '"';
            for (char c : str) {
                switch (c) {
                case '\n':
                    ctx_.out << "\\n";
                    break;
                case '\r':
                    ctx_.out << "\\r";
                    break;
                case '\t':
                    ctx_.out << "\\t";
                    break;
                case '"':
                    ctx_.out << "\\\"";
                    break;
                case '\\':
                    ctx_.out << "\\\\";
                    break;
                default:
                    ctx_.out << c;
                }
            }
            ctx_.out << '"';
        }

        void operator()(const Array& array) {
            if (array.empty()) {
                ctx_.out << "[]";
                return;
            }

            ctx_.out << "[\n";
            bool first = true;
            auto inner_ctx = ctx_.Indented();
            for (const Node& elem : array) {
                if (!first) {
                    ctx_.out << ",\n";
                }
                first = false;
                inner_ctx.PrintIndent();
                visit(PrintNode{ inner_ctx }, elem.GetValue());
            }
            ctx_.out << "\n";
            ctx_.PrintIndent();
            ctx_.out << "]";
        }

        void operator()(const Dict& dict) {
            if (dict.empty()) {
                ctx_.out << "{}";
                return;
            }

            ctx_.out << "{\n";
            bool first = true;
            auto inner_ctx = ctx_.Indented();
            for (const auto& [key, value] : dict) {
                if (!first) {
                    ctx_.out << ",\n";
                }
                first = false;
                inner_ctx.PrintIndent();
                ctx_.out << "\"" << key << "\": ";
                visit(PrintNode{ inner_ctx }, value.GetValue());
            }
            ctx_.out << "\n";
            ctx_.PrintIndent();
            ctx_.out << "}";
        }

    private:
        PrintContext ctx_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }
}  // namespace json