#pragma once
/*
    1) Класс Cell, наследуемый от CellInterface, который представляет ячейку в таблице.

    2) Внутри класса Cell определен интерфейс Impl, который абстрагирует различные типы содержимого ячейки.

    3) Класс Cell содержит три реализации интерфейса Impl: EmptyImpl, TextImpl и FormulaImpl. EmptyImpl представляет пустую ячейку, 
    TextImpl представляет текстовую ячейку с простым текстом, а FormulaImpl представляет формульную ячейку с вычисляемым выражением.

    4) Внутри класса Cell есть поля:
        impl_: уникальный указатель на объект, реализующий интерфейс Impl, который представляет содержимое ячейки.
        sheet_: ссылка на объект SheetInterface, к которому принадлежит эта ячейка.
        cache_: опциональное значение, которое кэширует результат последнего вычисления содержимого ячейки.
        calculated_cells_: множество указателей на ячейки, которые используют текущую ячейку в формуле.
        using_cells_: множество указателей на ячейки, на которые ссылается текущая ячейка через формулу.

    5) Методы ResetCache(), GetValue() и GetText(), которые реализуют интерфейс CellInterface, а также методы для обработки формул (Set()) и очистки ячейки (Clear()).
*/
#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
    public:
        Cell();
        ~Cell();

        // Устанавливает текст для ячейки и обновляет ее значение.
        void Set(std::string text);

        // Очищает содержимое ячейки.
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;

    private:
        // Интерфейс для реализации различных типов содержимого ячейки.
        class Impl {
            public:
                virtual Value GetValue() const = 0;
                virtual std::string GetText() const = 0;
                virtual std::vector<Position> GetReferencedCells() const = 0;
                virtual ~Impl() = default;
        };

        // Реализация для пустой ячейки.
        class EmptyImpl : public Impl {
            public:
                Value GetValue() const override;
                std::string GetText() const override;
                std::vector<Position> GetReferencedCells() const override;
        };

        // Реализация для текстовой ячейки.
        class TextImpl : public Impl {
            public:
                explicit TextImpl(std::string text);
                Value GetValue() const override;
                std::string GetText() const override;
                std::vector<Position> GetReferencedCells() const override;

            private:
                std::string text_;
        };

        // Реализация для формульной ячейки.
        class FormulaImpl : public Impl {
            public:
                explicit FormulaImpl(std::string text, const SheetInterface& sheet);
                Value GetValue() const override;
                std::string GetText() const override;
                std::vector<Position> GetReferencedCells() const override;

            private:
                std::unique_ptr<FormulaInterface> formula_ptr_;
        };

        void ResetCache();

        std::unique_ptr<Impl> impl_;
        SheetInterface& sheet_;
        mutable std::optional<Value> cache_;

        std::set<Cell*> calculated_cells_;
        std::set<Cell*> using_cells_;
};

