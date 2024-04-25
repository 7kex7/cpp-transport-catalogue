#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <optional>

#include "json.h"

namespace json {

class Builder {
    enum class Commands {
        NONE,
        KEY,
        VALUE,
        START_DICT,
        END_DICT,
        START_ARRAY,
        END_ARRAY,
    };

private:
    std::optional<Node> root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
    std::vector<std::string> key_stack_;
    Commands last_cmd = Commands::NONE;

    void AddToArray(Node& container_node, std::unique_ptr<Node>&& value_ptr);
    void AddToDict(Node& container_node, const Node&& value);
    void EndContainer();

public:
    struct DictItemContext;
    
    class DictNextElementContext { // after StartDict
        Builder& builder;
    public:
        DictNextElementContext(Builder& b) : builder(b) {}
        DictItemContext Key(const std::string& key) {
            return DictItemContext{builder.Key(key)};
        }
        Builder& EndDict() {
            return builder.EndDict();
        }
    };

    class ArrayNextElementContext { // after StartArray
        Builder& builder;
    public:
        ArrayNextElementContext(Builder& b) : builder(b) {}
        ArrayNextElementContext Value(const Node::Value& value) {
            return ArrayNextElementContext{builder.Value(value)};
        }
        DictNextElementContext StartDict() {
            return DictNextElementContext{builder.StartDict()};
        }
        ArrayNextElementContext StartArray() {
            return ArrayNextElementContext{builder.StartArray()};
        }
        Builder& EndArray() {
            return builder.EndArray();
        }
    };

    class DictItemContext { // after Key
        Builder& builder;
    public:
        DictItemContext(Builder& b) : builder(b) {}
        DictNextElementContext Value(const Node::Value& value) {
            return DictNextElementContext{builder.Value(value)};
        }
        DictNextElementContext StartDict() {
            return DictNextElementContext{builder.StartDict()};
        }
        ArrayNextElementContext StartArray() {
            return ArrayNextElementContext{builder.StartArray()};
        }
    };

    Builder();

    ArrayNextElementContext StartArray();
    Builder& EndArray();

    DictNextElementContext StartDict();
    Builder& EndDict();

    DictItemContext Key(std::string key);

    Builder& Value(Node::Value value);

    Node& Build();
};

} // json
