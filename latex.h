#pragma once

#include "vector_extesions.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

inline std::string to_string(std::string_view string_view)
{
    return { string_view.begin(), string_view.end() };
}

void write_preamble(FILE* dst)
{
    fprintf(dst, "\\documentclass[a2paper, 9pt]{article}\n"
                 "\\usepackage[T2A]{fontenc}\n"
                 "\\usepackage[utf8]{inputenc}\n"
                 "\\usepackage[english,russian]{babel}\n"
                 "\\usepackage{graphicx}\n"
                 "\\usepackage{xcolor}\n"
                 "\\usepackage{color}\n"
                 "\\usepackage{hyperref}\n"
                 "\\usepackage{mathtext}\n"
                 "\\usepackage{amsmath}\n"
                 "\\usepackage{amssymb}\n"
                 "\\usepackage{multirow}\n"
                 "\\DeclareGraphicsExtensions{.png,.jpg,.svg,.pdf}\n"
                 "\\usepackage[left=10mm,right=10mm,top=10mm,bottom=15mm]{geometry}\n"
                 "\\newcommand{\\TRUE}{\\mathtt{true}}\n"
                 "\\newcommand{\\FALSE}{\\mathtt{false}}\n"
                 "\\newcommand{\\NEXT}{\\mathbf{X}}\n"
                 "\\newcommand{\\FUTURE}{\\mathbf{F}}\n"
                 "\\newcommand{\\GLOBALLY}{\\mathbf{G}}\n"
                 "\\newcommand{\\UNTIL}{\\hspace{0.1cm}\\mathbf{U}\\hspace{0.1cm}}\n"
                 "\\newcommand{\\WEAK}{\\hspace{0.1cm}\\mathbf{W}\\hspace{0.1cm}}\n"
                 "\\newcommand{\\RELEASE}{\\hspace{0.1cm}\\mathbf{R}\\hspace{0.1cm}}\n"
                 "\\newcommand{\\NOT}{\\mathbf{\\neg}}\n"
                 "\\newcommand{\\AND}{\\hspace{0.1cm}\\mathbf{\\wedge}\\hspace{0.1cm}}\n"
                 "\\newcommand{\\OR}{\\hspace{0.1cm}\\mathbf{\\vee}\\hspace{0.1cm}}\n"
                 "\\newcommand{\\IMPL}{\\hspace{0.1cm}\\mathbf{\\rightarrow}\\hspace{0.1cm}}\n"
                 "\\newcommand\\Warning{%\n"
                 " \\makebox[1.4em][c]{%\n"
                 " \\makebox[0pt][c]{\\raisebox{.1em}{\\Large\\textbf{!}}}%\n"
                 " \\makebox[0pt][c]{\\color{red}\\Huge$\\bigtriangleup$}}}%\n"
                 "\\begin{document}\n"
                 "    \\begin{figure}[!htb]\n"
                 "        \\centering\n"
                 "        \\noindent\\hrulefill\n"
                 "        \\vspace{0.5cm}\n\n"
                 "        \\begin{minipage}{0.07\\textwidth}\n"
                 "            \\Warning\n"
                 "        \\end{minipage}%\n"
                 "        \\begin{minipage}{0.93\\textwidth}\n"
                 "            Решение, приведенное в данном документе, не претендует на стопроцентную точность, будьте внимательны и аккуратны (а лучше проверяйте решение и репортите ошибки)\n"
                 "        \\end{minipage}\n\n"
                 "        \\vspace{0.4cm}\n"
                 "        \\noindent\\hrulefill\n"
                 "    \\end{figure}\n\n");
}

void write_ending(FILE* dst)
{
    fprintf(dst, "\n\\end{document}\n");
}


class TeXDocument
{
public:
    class TeXPackage
    {
    private: 
        std::string name;
        std::vector<std::string> options;
        std::map<std::string, std::string> named_options;

    public:
        TeXPackage(
            std::string_view package_name,
            std::vector<std::string_view> options_list = {},
            std::map<std::string_view, std::string_view> named_options_map = {}) : name(package_name) 
        {
            for (auto option : options_list)
                options.push_back(to_string(option));

            for (auto& [option_name, option_value] : named_options_map)
                named_options[to_string(option_name)] = to_string(option_value);
        }
        
        void add_parameter(std::string_view option, std::string_view named_option_value = nullptr)
        {
            if (named_option_value.empty())
                options.push_back(to_string(option));
            else
                named_options[to_string(option)] = to_string(named_option_value);
        }

        bool operator==(const TeXPackage& other)
        {
            if (name != other.name || options.size() != other.options.size() || named_options.size() != other.named_options.size())
                return false;

            return true;
        }
    };

private:

    std::string document_class;
    std::string paper_size;
    std::string font_size;
    std::vector<TeXPackage> packages;

public:

    TeXDocument(std::string_view doc_class, std::string_view paper_size = nullptr, std::string_view font_size = nullptr) : document_class(doc_class), paper_size(paper_size), font_size(font_size) { }

    void add_package(TeXPackage package)
    {
        add_if_not_presented(packages, package);
    }

    void add_package(std::string_view package_name, 
        std::vector<std::string_view> options_list = {},
        std::map<std::string_view, std::string_view> named_options_map = {})
    {
        add_if_not_presented(packages, TeXPackage(package_name, options_list, named_options_map));
    }

    void write_to(FILE* f)
    {
    }
};
