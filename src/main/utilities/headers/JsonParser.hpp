/**
 * @file JsonParser.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <string>
#include <map>
#include <vector>
#include <cassert>
#include <cstdio>

using std::string;
using std::map;
using std::vector;

enum JsonType {
    DATA,
    OBJECT,
    LIST
};

struct JsonNode {
    map<string, JsonNode *> objectData;
    JsonType type;
    vector<JsonNode> listData;
    string data;
};

JsonNode *parseJsonHelper(std::string::iterator *it) {
    printf("Current char: %c\n", **it);
    auto doReturn = false;
    auto node = new JsonNode();
    auto inString = false;
    auto inKey = true;
    string key = "";
    string value = "";
    while (!doReturn) {
        auto isSpace = false;
        ++it;
        switch (**it) {
            case '{':
                printf("Creating new object");
                // Recursively call this function
                // Assert if the key length is empty
                assert(!key.empty());
                node->objectData[key] = parseJsonHelper(it);
                break;
            case '}':
                doReturn = true;
            case ',':
                if (!value.empty()) {
                    // Save the value as json node
                    node->objectData[key] = new JsonNode();
                    node->objectData[key]->data = value;
                }
                break;
            case ':':
                inKey = false;
                break;
            case '\"':
                inString = !inString;
                break;
            case ' ':
                isSpace = true;
                // Ignore spaces outside of key
            default:
                if (inString && inKey) {
                    key += **it;
                } else if (!inKey && !isSpace) {
                    value += **it;
                }
                // Parse name from chars
        }
    }
    printf("Key: %s\n", key.c_str());
    printf("Value: %s\n", value.c_str());
    return node;
}

//" { } ` ` [ ] , are special characters
JsonNode *parseJson(std::string jsonData) {
    auto it = jsonData.begin();
    assert(*it == '{');
    auto node = parseJsonHelper(&it);
    return node;
}

/*
//" { } ` ` [ ] , are special characters
JsonNode *parseJson(std::string jsonData) {
    auto it = jsonData.begin();
    JsonNode *node = new JsonNode();
    int objectLevel = 0;
    int listLevel = 0;
    StateMachine chState = SPACE;
    // Iterate through each character and parse json
    for (auto ch : jsonData) {
        assert(ch == '{' || objectLevel > 0);
        switch (ch) {
            case '{':
                // Increase the objectLevel
                objectLevel++;
                break;
            case '}':
                objectLevel--;
                break;
            case '[':
                listLevel++;
                break;
            case ']':
                listLevel--;
                break;
            case '\n':
            case '\r':
            case ' ':
                printf("Skipping space");
                break;
            case ',':
                printf("COMMA\n");
                break;
            default:
                printf("accepting ASCII %c\n", ch);
                break;
        }
    }
    if (jsonData.empty()) {
        return nullptr;
    }
    return node;
}
*/
