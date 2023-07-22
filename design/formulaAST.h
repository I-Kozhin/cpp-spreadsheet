#pragma once
/*
    1)  Класс FormulaAST, представляющий синтаксическое дерево формулы. Этот класс содержит указатель на корневое выражение (Expr), 
    которое является базовым классом для всех узлов дерева.

    2)  Класс FormulaAST имеет конструктор, который принимает уникальный указатель на корневое выражение, 
    и методы для выполнения формулы, вывода дерева и вывода формулы в текстовом виде.

    3)  Класс ParsingError, наследуемый от std::runtime_error, который представляет исключение, возникающее при синтаксических 
    ошибках при разборе формулы.

    4)  Функции ParseFormulaAST(std::istream& in) и ParseFormulaAST(const std::string& in_str), которые разбирают формулу из потока 
    или строки и создают объект FormulaAST. Если формула содержит синтаксические ошибки, эти функции могут сгенерировать исключение 
    ParsingError.
*/
#include "FormulaLexer.h"
#include "common.h"
  
#include <forward_list>
#include <functional>
#include <stdexcept>

namespace ASTImpl {
    class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Класс FormulaAST представляет синтаксическое дерево формулы.
// Он обеспечивает выполнение формулы и отображение дерева.
class FormulaAST {
    public:
        explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr);
        FormulaAST(FormulaAST&&) = default;
        FormulaAST& operator=(FormulaAST&&) = default;
        ~FormulaAST();

        // Выполняет формулу, используя переданную функцию для получения значений ячеек.
        // Возвращает результат вычисления формулы.
        double Execute(std::function<double(Position* pos)> func) const;

        // Выводит дерево формулы в указанный поток.
        void Print(std::ostream& out) const;

        // Выводит формулу в указанный поток в виде текста.
        void PrintFormula(std::ostream& out) const;

        // Возвращает список позиций ячеек, используемых в формуле.
        // Необходим для определения зависимостей между ячейками.
        std::list<Position>& GetCells();

    private:
        std::unique_ptr<ASTImpl::Expr> root_expr_;
        std::list<Position> cells_;
};

// Функции для разбора формулы и создания объекта FormulaAST.
// Могут генерировать исключение ParsingError, если формула содержит синтаксические ошибки.
FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);
