#pragma once
/*
    1) Класс Sheet, наследуемый от SheetInterface, который представляет таблицу с ячейками.

    2) Используется тип Table, который представляет двумерный вектор с уникальными указателями на ячейки, 
    чтобы хранить содержимое таблицы.

    3) Класс Sheet содержит методы для работы с ячейками, такие как SetCell(), GetCell(), ClearCell(), 
    GetPrintableSize(), PrintValues() и PrintTexts().
*/
#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>

// Используется для хранения ячеек таблицы.
using Table = std::vector<std::vector<std::unique_ptr<Cell>>>;

// Класс Sheet представляет таблицу с ячейками.
class Sheet : public SheetInterface {
    public:
        ~Sheet();

        // Устанавливает текст для ячейки по указанной позиции.
        // Если ячейки с такой позицией не существует, она будет создана.
        void SetCell(Position pos, std::string text) override;

        // Возвращает указатель на ячейку по указанной позиции.
        // Если ячейки с такой позицией не существует, возвращает nullptr.
        const CellInterface* GetCell(Position pos) const override;

        // Возвращает указатель на ячейку по указанной позиции.
        // Если ячейки с такой позицией не существует, возвращает nullptr.
        CellInterface* GetCell(Position pos) override;

        // Очищает содержимое ячейки по указанной позиции.
        void ClearCell(Position pos) override;

        // Возвращает размер таблицы, который может быть напечатан (включая пустые ячейки).
        Size GetPrintableSize() const override;

        // Выводит значения ячеек в указанный поток.
        void PrintValues(std::ostream& output) const override;

        // Выводит тексты ячеек в указанный поток.
        void PrintTexts(std::ostream& output) const override;

    private:
        Table cells_;
};
