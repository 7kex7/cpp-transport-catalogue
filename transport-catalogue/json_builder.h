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

public:
    struct ValueArrayContext;
    struct ValueDictContext;
    struct DictItemContext;
    
    struct DictInitContext { // after StartDict
        Builder& builder;
        DictItemContext Key(const std::string& key) {
            return DictItemContext{builder.Key(key)};
        }
        Builder& EndDict() {
            return builder.EndDict();
        }
    };

    struct ArrayInitContext { // after StartArray
        Builder& builder;
        ValueArrayContext Value(const Node::Value& value) {
            return ValueArrayContext{builder.Value(value)};
        }
        DictInitContext StartDict() {
            return DictInitContext{builder.StartDict()};
        }
        ArrayInitContext StartArray() {
            return ArrayInitContext{builder.StartArray()};
        }
        Builder& EndArray() {
            return builder.EndArray();
        }
    };

    struct DictItemContext { // after Key
        Builder& builder;
        ValueDictContext Value(const Node::Value& value) {
            return ValueDictContext{builder.Value(value)};
        }
        DictInitContext StartDict() {
            return DictInitContext{builder.StartDict()};
        }
        ArrayInitContext StartArray() {
            return ArrayInitContext{builder.StartArray()};
        }
    };

    // after value in array
    struct ValueArrayContext : public ArrayInitContext {
    };

    // after value in dictionary
    struct ValueDictContext : public DictInitContext {
    };

    Builder();

    void AddToArray(Node& container_node, std::unique_ptr<Node>&& value_ptr);
    void AddToDict(Node& container_node, Node& value);

    ArrayInitContext StartArray();
    Builder& EndArray();

    DictInitContext StartDict();
    Builder& EndDict();

    DictItemContext Key(const std::string& key);

    Builder& Value(const Node::Value& value);

    Node& Build();
};

} // json
