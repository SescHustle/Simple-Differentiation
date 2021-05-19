#include<iostream>
#include<fstream>
#include<string>

using namespace std;

class Expression {
public:
    Expression() {};
    virtual Expression *copy() = 0;
    virtual Expression* diff() = 0;
    virtual ~Expression() {};
    virtual void print(ofstream &fout) = 0;
};

class Number : public Expression {
private:
    int value;

public:
    Number(int num) : value(num) {}

    Expression *diff() override
    {
        Number *a = new Number(this->value);
        a->value = 0;
        return a;
    }

    Expression *copy() override
    {
        return new Number(this->value);
    }

    void print(ofstream &fout) override
    {
        fout << this->value;
    }

    ~Number() override {}
};

class Variable : public Expression {
private:
    string value;

public:
    Variable(string var) : value(var) {}

    Expression *copy() override
    {
        return new Variable(this->value);
    }

    Expression *diff() override
    {
        Variable *a = new Variable(this->value);
        a->value = '1';
        return a;
    }

    void print(ofstream &fout) override
    {
        fout << this->value;
    }

    ~Variable() override {};
};

class Add : public Expression{
protected:
    Expression *firstAdd;
    Expression *secondAdd;

public:
    Add(Expression *first, Expression *second)
    {
        this->firstAdd = first;
        this->secondAdd = second;
    }

    Expression *diff() override
    {
        return new Add(this->firstAdd->diff(), this->secondAdd->diff());
    }

    Expression *copy() override
    {
        return new Add(this->firstAdd->copy(), this->secondAdd->copy());
    }

    void print(ofstream &fout) override
    {
        fout << "(";
        this->firstAdd->print(fout);
        fout << "+";
        this->secondAdd->print(fout);
        fout << ")";
    }

    ~Add()
    {
        delete this->firstAdd;
        delete this->secondAdd;
    }
};

class Sub : public Expression {
protected:
    Expression *firstSub;
    Expression *secondSub;

public:
    Sub(Expression *first, Expression *second)
    {
        this->firstSub = first;
        this->secondSub = second;
    }

    Expression *copy() override
    {
        return new Sub(this->firstSub->copy(), this->secondSub->copy());
    }

    Expression *diff() override
    {
        return new Sub(this->firstSub->diff(), this->secondSub->diff());
    }

    void print(ofstream &fout) override
    {
        fout << "(";
        this->firstSub->print(fout);
        fout << "-";
        this->secondSub->print(fout);
        fout << ")";
    }

    ~Sub()
    {
        delete this->firstSub;
        delete this->secondSub;
    }
};

class Mul : public Expression {
protected:
    Expression *firstMul;
    Expression *secondMul;

public:
    Mul(Expression *first, Expression *second)
    {
        this->firstMul = first;
        this->secondMul = second;
    }

    Expression *copy() override
    {
        return new Mul(this->firstMul->copy(), this->secondMul->copy());
    }

    Expression *diff() override
    {
        return new Add(new Mul(this->firstMul->diff(), this->secondMul->copy()),
                       new Mul(this->firstMul->copy(), this->secondMul->diff()));
    }

    void print(ofstream &fout) override
    {
        fout << "(";
        this->firstMul->print(fout);
        fout << "*";
        this->secondMul->print(fout);
        fout << ")";
    }

    ~Mul()
    {
        delete this->firstMul;
        delete this->secondMul;
    }
};

class Div : public Expression {
protected:
    Expression *numerator;
    Expression *denumerator;

public:
    Div(Expression *first, Expression *second)
    {
        this->numerator = first;
        this->denumerator = second;
    }

    Expression *copy()
    {
        return new Div(this->numerator->copy(), this->denumerator->copy());
    }

    Expression *diff() override
    {
        return new Div(new Sub(new Mul(this->numerator->diff(), this->denumerator->copy()),
                               new Mul(this->denumerator->diff(), this->numerator->copy())),
                       new Mul(this->denumerator->copy(), this->denumerator->copy()));
    }

    void print(std::ofstream &fout) override
    {
        fout << "(";
        this->numerator->print(fout);
        fout << "/";
        this->denumerator->print(fout);
        fout << ")";
    }

    ~Div()
    {
        delete this->numerator;
        delete this->denumerator;
    }
};

class Reader {
private:
    static Expression* NumberToExpression(const string & expr, int &position);
    static Expression* VariableToExpression(const string & expr, int &position);
    static Expression* InnerToExpression(const string & expr, int &position);
    static Expression* PartOfInnerToExpression(const string & expr, int &position);
    static Expression* SkipBracketsSymbols(const string & expr, int &position);

public:
    static Expression* StringToExpression(const string & expr);
};

Expression *Reader::NumberToExpression(const string &expr, int &position) {
    string tmp = "";
    int k = 0;

    for (position; isdigit(expr[position]); )
    {
        tmp[k++] = expr[position++];
    }

    return new Number(stoi(tmp));
}

Expression *Reader::VariableToExpression(const string &expr, int &position) {
    string tmp = "";
    int k = 0;

    for (position; isalpha(expr[position]) || expr[position] == '_'; )
    {
        tmp[k++] = expr[position++];
    }

    return new Variable(tmp);
}

Expression *Reader::InnerToExpression(const string & expr, int &position) {
    char operation;

    Expression *first = PartOfInnerToExpression(expr, position);
    SkipBracketsSymbols(expr, position);
    operation = expr[position++];
    Expression *second = PartOfInnerToExpression(expr, position);
    SkipBracketsSymbols(expr, position);

    switch (operation)
    {
        case '+': return new Add(first, second);
            break;
        case '-': return new Sub(first, second);
            break;
        case '*': return new Mul(first, second);
            break;
        case '/': return new Div(first, second);
            break;
        default:
            string err = "INCORRECT OPERATION SYMBOL " + operation;
            throw err;
    }
}

Expression *Reader::PartOfInnerToExpression(const string &expr, int &position)
{
    if (isdigit(expr[position]))
    {
        return NumberToExpression(expr, position);
    }
    else if (isalpha(expr[position]) || expr[position] == '_')
    {
        return VariableToExpression(expr, position);
    }
    else if (expr[position] == '(')
    {
        position++;
        return InnerToExpression(expr, position);
    }
    else
    {
        string err = "INCORRECT SYMBOL " + expr[position];
        throw err;
    }
}

Expression *Reader::SkipBracketsSymbols(const string & expr, int &position)
{
    while (expr[position] == '(' || expr[position] == ')')
    {
        position++;
    }
}

Expression *Reader::StringToExpression(const string &expr)
{
    int position = 0;

    if (isdigit(expr[position]))
    {
        return NumberToExpression(expr, position);
    }
    else if (isalpha(expr[position]) || expr[position] == '_')
    {
        return VariableToExpression(expr, position);
    }
    else if (expr[position] == '(')
    {
        return InnerToExpression(expr, ++position);
    }
    else
    {
        string err = "INCORRECT FIRST SYMBOL " + expr[position];
        throw err;
    }
}

int main()
{
    ifstream fin("input.txt");
    ofstream fout("output.txt");

    string expression = "";
    getline(fin, expression);

    try
    {
        Expression *e = Reader::StringToExpression(expression);
        Expression *de = e->diff();
        de->print(fout);
        delete e;
        delete de;
    }
    catch (const string &err)
    {
        fout << err;
    }

    fin.close();
    fout.close();
    return 0;
}
