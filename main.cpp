// GNU GPLv3 License (c)2024 Ivo Celmans

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <boost/multiprecision/mpfr.hpp>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <iomanip>

using namespace boost::multiprecision;

using type_big = number<mpfr_float_backend<999>>;

// Predefined values for custom functions
const type_big one ("1.0");
const type_big two ("2.0");
const type_big four ("4.0");
const type_big five ("5.0");
const type_big number180 ("180.0");
const type_big pi (atan(one) * four); // Generator of pi
const type_big e (exp(one)); // Generator of Eulers number
const type_big phi ((one + sqrt(five)) / two); // Generator of golden ratio

// Enum for operator types
enum class ElementOperator {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    Power,
    Modulus,
    OParent,
    CParent,
    Function,
    Constant,
    End
};

struct Element {
    ElementOperator type;
    std::string value;

    Element(ElementOperator t, const std::string& v) : type(t), value(v) {}
};

// Element function and notation error handling
std::vector<Element> elementCollector(const std::string& line) {
    std::vector<Element> elements;
    std::string::const_iterator it = line.begin();
    ElementOperator leo = ElementOperator::End; // leo - last element operator
    int parentCounter = 0;

    while (it != line.end()) {
        if (std::isspace(*it)) {
            ++it;
            continue;
        }

        if (std::isdigit(*it) || *it == '.') {
            std::string number;
            while (it != line.end() && (std::isdigit(*it) || *it == '.')) {
                number += *it++;
            }
                if (leo == ElementOperator::CParent || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }
            elements.emplace_back(ElementOperator::Number, number);
            leo = ElementOperator::Number;
            continue;
        }

        if (*it == 'e' && *(it + 1) != 'x') {
                if (leo == ElementOperator::CParent || leo == ElementOperator::Number || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }
            elements.emplace_back(ElementOperator::Constant, "e");
            it += 1;
            leo = ElementOperator::Constant;
            continue;
        }

        if (*it == 'p' && (it + 1) != line.end() && *(it + 1) == 'i') {
                if (leo == ElementOperator::CParent || leo == ElementOperator::Number || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }
            elements.emplace_back(ElementOperator::Constant, "pi");
            it += 2;
            leo = ElementOperator::Constant;
            continue;
        }

        if (*it == 'p' && (it + 1) != line.end() && *(it + 1) == 'h' && (it + 2) != line.end() && *(it + 2) == 'i') {
                if (leo == ElementOperator::CParent || leo == ElementOperator::Number || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }
            elements.emplace_back(ElementOperator::Constant, "phi");
            it += 3;
            leo = ElementOperator::Constant;
            continue;
        }

        if (std::isalpha(*it)) {
            std::string function;
            while (it != line.end() && std::isalpha(*it)) {
                function += *it++;
            }
                if (leo == ElementOperator::CParent || leo == ElementOperator::Number || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }           
            elements.emplace_back(ElementOperator::Function, function);
            leo = ElementOperator::Function;
            continue;
        }

        switch (*it) {
            case '+': elements.emplace_back(ElementOperator::Plus, "+"); leo = ElementOperator::Plus; break;
            case '-': elements.emplace_back(ElementOperator::Minus, "-"); leo = ElementOperator::Minus; break;
            case '*': elements.emplace_back(ElementOperator::Multiply, "*"); leo = ElementOperator::Multiply; break;
            case '/': elements.emplace_back(ElementOperator::Divide, "/"); leo = ElementOperator::Divide; break;
            case '^': elements.emplace_back(ElementOperator::Power, "^"); leo = ElementOperator::Power; break;
            case '%': elements.emplace_back(ElementOperator::Modulus, "%"); leo = ElementOperator::Modulus; break;
            case '(': 
                if (leo == ElementOperator::Number || leo == ElementOperator::CParent || leo == ElementOperator::Constant) {
                    throw std::runtime_error("Missing operator.");
                }
                elements.emplace_back(ElementOperator::OParent, "(");
                parentCounter++;
                leo = ElementOperator::OParent;
                break;
            case ')': 
                elements.emplace_back(ElementOperator::CParent, ")");
                parentCounter--;
                if (parentCounter < 0) {
                    throw std::runtime_error("Mismatched parentheses.");
                }
                leo = ElementOperator::CParent;
                break;
            default: throw std::runtime_error("Unknown character: " + std::string(1, *it));
        }
        ++it;
    }
    elements.emplace_back(ElementOperator::End, "");
    return elements;
}

// Base class for line nodes
class LineNode {
public:
    virtual ~LineNode() {}
    virtual type_big evaluate() const = 0;
};

// Node for numbers
class NumberNode : public LineNode {
    type_big value;
public:
    NumberNode(const type_big& val) : value(val) {}
    type_big evaluate() const override { return value; }
};

// Node for constants
class ConstantNode : public LineNode {
    type_big value;
public:
    ConstantNode(const std::string& name) {
        if (name == "pi") {
            value = pi;
        } else if (name == "e"){
            value = e;
        } else if (name == "phi"){
            value = phi;
        } else {
            throw std::runtime_error("Unknown constant: " + name);
        }
    }
    type_big evaluate() const override { return value; }
};

// Node for unary operations
class UnaryOpNode : public LineNode {
protected:
    std::unique_ptr<LineNode> oper;

public:
    UnaryOpNode(LineNode* op) : oper(op) {}
};

// Node for unary minus
class UnaryMinusNode : public UnaryOpNode {
public:
    UnaryMinusNode(LineNode* op) : UnaryOpNode(op) {}
    type_big evaluate() const override {
        return -oper->evaluate();
    }
};

// Node for unary plus
class UnaryPlusNode : public UnaryOpNode {
public:
    UnaryPlusNode(LineNode* op) : UnaryOpNode(op) {}
    type_big evaluate() const override {
        return oper->evaluate();
    }
};

// Node for binary operations
class BinaryOpNode : public LineNode {
protected:
    std::unique_ptr<LineNode> left;
    std::unique_ptr<LineNode> right;

public:
    BinaryOpNode(LineNode* l, LineNode* r) : left(l), right(r) {}
};

// Node for addition
class AddNode : public BinaryOpNode {
public:
    AddNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        return left->evaluate() + right->evaluate();
    }
};

// Node for subtraction
class SubtractNode : public BinaryOpNode {
public:
    SubtractNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        return left->evaluate() - right->evaluate();
    }
};

// Node for multiply
class MultiplyNode : public BinaryOpNode {
public:
    MultiplyNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        return left->evaluate() * right->evaluate();
    }
};

// Node for division
class DivideNode : public BinaryOpNode {
public:
    DivideNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        type_big denom = right->evaluate();
        if (denom == 0) throw std::runtime_error("Division by zero.");
        return left->evaluate() / denom;
    }
};

// Node for Power
class PowerNode : public BinaryOpNode {
public:
    PowerNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        return pow(left->evaluate(), right->evaluate());
    }
};

// Node for Modulus
class ModulusNode : public BinaryOpNode {
public:
    ModulusNode(LineNode* l, LineNode* r) : BinaryOpNode(l, r) {}
    type_big evaluate() const override {
        return fmod(left->evaluate(), right->evaluate());
    }
};

// Node for function calls
class FunctionNode : public LineNode {
    std::string functionName;
    std::unique_ptr<LineNode> argument;

public:
    FunctionNode(const std::string& name, LineNode* arg) 
        : functionName(name), argument(arg) {}

    type_big evaluate() const override {
        type_big argValue = argument->evaluate();
        if (functionName == "sin") return sin(argValue);
        if (functionName == "cos") return cos(argValue);
        if (functionName == "tan") return tan(argValue);
        if (functionName == "sinh") return sinh(argValue);
        if (functionName == "cosh") return cosh(argValue);
        if (functionName == "tanh") return tanh(argValue);
        if (functionName == "ln") return log(argValue);
        if (functionName == "log") return log10(argValue);
        if (functionName == "exp") return exp(argValue);
        if (functionName == "sqrt") return sqrt(argValue);
        // Customized functions
        if (functionName == "sind") return sin(argValue * pi / number180);
        if (functionName == "cosd") return cos(argValue * pi / number180);
        if (functionName == "tand") return tan(argValue * pi / number180);
        if (functionName == "fact") return tgamma(argValue + one);
        if (functionName == "rms") return (argValue / sqrt(two));
        if (functionName == "peak") return (argValue * sqrt(two));
        throw std::runtime_error("Unknown function: " + functionName);
    }
};

// Forward declarations for calculating functions
class calculator {
    std::vector<Element> elements;
    size_t current;

    Element currentElement() {
        return elements[current];
    }

    void progress() {
        current++;
    }

    LineNode* processLine();
    LineNode* processLaw();
    LineNode* processFactor();
    LineNode* processFunction();

public:
    calculator(const std::vector<Element>& ts) : elements(ts), current(0) {}

    LineNode* process() {
        return processLine();
    }
};

// Parsing logic
LineNode* calculator::processLine() {
    LineNode* left = processLaw();
    
    while (currentElement().type == ElementOperator::Plus || currentElement().type == ElementOperator::Minus) {
        Element op = currentElement();
        progress();
        LineNode* right = processLaw();
        
        if (op.type == ElementOperator::Plus) {
            left = new AddNode(left, right);
        } else {
            left = new SubtractNode(left, right);
        }
    }
    return left;
}

LineNode* calculator::processLaw() {
    LineNode* left = processFactor();
    
    while (currentElement().type == ElementOperator::Multiply || 
           currentElement().type == ElementOperator::Divide || 
           currentElement().type == ElementOperator::Modulus) {
        Element op = currentElement();
        progress();
        LineNode* right = processFactor();
        
        if (op.type == ElementOperator::Multiply) {
            left = new MultiplyNode(left, right);
        } else if (op.type == ElementOperator::Divide) {
            left = new DivideNode(left, right);
        } else if (op.type == ElementOperator::Modulus) {
            left = new ModulusNode(left, right);
        }
    }
    
    while (currentElement().type == ElementOperator::Power) {
        Element op = currentElement();
        progress();
        LineNode* right = processFactor();
        left = new PowerNode(left, right);
    }
    
    return left;
}

LineNode* calculator::processFactor() {
    if (currentElement().type == ElementOperator::Minus) {
        progress();
        LineNode* oper = processFactor();
        return new UnaryMinusNode(oper);
    }

    if (currentElement().type == ElementOperator::Plus) {
        progress();
        LineNode* oper = processFactor();
        return new UnaryPlusNode(oper);
    }

    if (currentElement().type == ElementOperator::Number) {
        type_big value(currentElement().value);
        progress();
        return new NumberNode(value);
    }

    if (currentElement().type == ElementOperator::Constant) {
        std::string constantName = currentElement().value;
        progress();
        return new ConstantNode(constantName);
    }

    if (currentElement().type == ElementOperator::Function) {
        std::string functionName = currentElement().value;
        progress();
        if (currentElement().type != ElementOperator::OParent) {
            throw std::runtime_error("Missing function argument.");
        }
        progress();
        LineNode* argument = processLine();
        if (currentElement().type != ElementOperator::CParent) {
            throw std::runtime_error("Mismatched parentheses.");
        }
        progress();
        return new FunctionNode(functionName, argument);
    }

    if (currentElement().type == ElementOperator::OParent) {
        progress();
        LineNode* node = processLine();
        if (currentElement().type != ElementOperator::CParent) {
            throw std::runtime_error("Mismatched parentheses.");
        }
        progress();
        return node;
    }

    throw std::runtime_error("Input Invalid.");
}

int main() {
    while(true){
        std::string input;
        std::cout << "> ";
        std::getline(std::cin >> std::ws, input);
            if (input == "exit"){ break; }
        try {
            auto elements = elementCollector(input);
            calculator calculate(elements);
            std::unique_ptr<LineNode> root(calculate.process());
            type_big result = root->evaluate();
            std::cout << "  " << std::setprecision(999) << result << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}
