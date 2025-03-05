#pragma once

#include <string>
#include <vector>

namespace Arcanelab::Mano
{
    class ErrorReporter
    {
    public:
        enum class Phase { Lexer, Parser, Semantic };
        enum class Severity { Error, Warning };

        // Constructor binds to specific phase
        explicit ErrorReporter(Phase phase) : m_phase(phase) {}

        void Report(size_t line, size_t column,
            const std::string& message,
            Severity severity = Severity::Error)
        {
            m_errors.push_back({ line, column, message, severity, m_phase });
        }

        // Accessors
        Phase GetPhase() const { return m_phase; }
        const auto& GetErrors() const { return m_errors; }
        bool HasErrors() const { return !m_errors.empty(); }

    private:
        struct ErrorEntry
        {
            size_t line;
            size_t column;
            std::string message;
            Severity severity;
            Phase phase;
        };

        Phase m_phase;
        std::vector<ErrorEntry> m_errors;
    };
} // namespace
