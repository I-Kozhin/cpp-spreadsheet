#pragma once

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

/**
 * @brief Класс FormulaAST представляет абстрактное синтаксическое дерево (AST) для формулы.
 * 
 * Формула представляется в виде дерева, где каждый узел представляет операнд или операцию.
 * Класс предоставляет методы для выполнения вычислений и отладочного вывода AST.
 */
class FormulaAST {
    public:
        /**
         * @brief Конструктор класса FormulaAST.
         * 
         * @param root_expr Указатель на корневой узел AST.
         * @param cells Список позиций ячеек, на которые ссылается формула.
         */
        explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                            std::forward_list<Position> cells);

        FormulaAST(FormulaAST&&) = default;
        FormulaAST& operator=(FormulaAST&&) = default;
        ~FormulaAST();

        /**
         * @brief Выполняет вычисление формулы с заданными аргументами.
         * 
         * Метод принимает функцию, которая возвращает значение ячейки по ее позиции.
         * Вычисление выполняется рекурсивно, начиная с корневого узла AST.
         * 
         * @param args Функция, возвращающая значение ячейки по ее позиции.
         * @return Результат вычисления формулы.
         */
        double Execute(const std::function<double(Position)>& args) const;

        /**
         * @brief Выводит список позиций ячеек, на которые ссылается формула.
         * 
         * Метод выводит список позиций ячеек, которые были использованы при построении AST формулы.
         * 
         * @param out Поток вывода, в который будет выведен список позиций ячеек.
         */
        void PrintCells(std::ostream& out) const;

        /**
         * @brief Выводит AST формулы в поток вывода.
         * 
         * Метод выводит абстрактное синтаксическое дерево формулы в поток вывода в виде текста.
         * 
         * @param out Поток вывода, в который будет выведен AST формулы.
         */
        void Print(std::ostream& out) const;

        /**
         * @brief Выводит строковое представление формулы в поток вывода.
         * 
         * Метод выводит строковое представление формулы в поток вывода в виде текста.
         * 
         * @param out Поток вывода, в который будет выведено строковое представление формулы.
         */
        void PrintFormula(std::ostream& out) const;

        /**
         * @brief Возвращает ссылку на список позиций ячеек, на которые ссылается формула.
         * 
         * @return Ссылка на список позиций ячеек, на которые ссылается формула.
         */
        std::forward_list<Position>& GetCells() { return cells_; }

        /**
         * @brief Возвращает константную ссылку на список позиций ячеек, на которые ссылается формула.
         * 
         * @return Константная ссылка на список позиций ячеек, на которые ссылается формула.
         */
        const std::forward_list<Position>& GetCells() const { return cells_; }

    private:
        std::unique_ptr<ASTImpl::Expr> root_expr_;  /*< Указатель на корневой узел AST. */
        std::forward_list<Position> cells_;        /*< Список позиций ячеек, на которые ссылается формула. */
};

/**
 * @brief Функция для разбора формулы и создания объекта FormulaAST.
 * 
 * @param in Поток ввода, содержащий формулу в текстовом виде.
 * @return Объект FormulaAST, представляющий абстрактное синтаксическое дерево формулы.
 */
FormulaAST ParseFormulaAST(std::istream& in);

/**
 * @brief Функция для разбора формулы и создания объекта FormulaAST.
 * 
 * @param in_str Строка с формулой в текстовом виде.
 * @return Объект FormulaAST, представляющий абстрактное синтаксическое дерево формулы.
 */
FormulaAST ParseFormulaAST(const std::string& in_str);
