#pragma once

#include "latex.h"

class Solver
{
private:
    static void write_preamble(TeXDocument& document)
    {
        document.add_package("fontenc", {"T2A"});
        document.add_package("inputenc", {"utf8"});
        document.add_package("babel", {"english", "russian"});
        document.add_package("graphicx");
        document.add_package("xcolor");
        document.add_package("color");
        document.add_package("hyperref");
        document.add_package("mathtext");
        document.add_package("amsmath");
        document.add_package("amssymb");
        document.add_package("multirow");
        document.add_package("geometry", {}, {{"left", "10mm"}, {"right", "10mm"}, {"top", "10mm"}, {"bottom", "15mm"}});

        // "\\DeclareGraphicsExtensions{.png,.jpg,.svg,.pdf}\n"
        // "\\usepackage[left=10mm,right=10mm,top=10mm,bottom=15mm]{geometry}\n"
        // "\\newcommand{\\TRUE}{\\mathtt{true}}\n"
        // "\\newcommand{\\FALSE}{\\mathtt{false}}\n"
        // "\\newcommand{\\NEXT}{\\mathbf{X}}\n"
        // "\\newcommand{\\FUTURE}{\\mathbf{F}}\n"
        // "\\newcommand{\\GLOBALLY}{\\mathbf{G}}\n"
        // "\\newcommand{\\UNTIL}{\\hspace{0.1cm}\\mathbf{U}\\hspace{0.1cm}}\n"
        // "\\newcommand{\\WEAK}{\\hspace{0.1cm}\\mathbf{W}\\hspace{0.1cm}}\n"
        // "\\newcommand{\\RELEASE}{\\hspace{0.1cm}\\mathbf{R}\\hspace{0.1cm}}\n"
        // "\\newcommand{\\NOT}{\\mathbf{\\neg}}\n"
        // "\\newcommand{\\AND}{\\hspace{0.1cm}\\mathbf{\\wedge}\\hspace{0.1cm}}\n"
        // "\\newcommand{\\OR}{\\hspace{0.1cm}\\mathbf{\\vee}\\hspace{0.1cm}}\n"
        // "\\newcommand{\\IMPL}{\\hspace{0.1cm}\\mathbf{\\rightarrow}\\hspace{0.1cm}}\n"
        // "\\newcommand\\Warning{%\n"
        // " \\makebox[1.4em][c]{%\n"
        // " \\makebox[0pt][c]{\\raisebox{.1em}{\\Large\\textbf{!}}}%\n"
        // " \\makebox[0pt][c]{\\color{red}\\Huge$\\bigtriangleup$}}}%\n"
        // "\\begin{document}\n"
        // "    \\begin{figure}[!htb]\n"
        // "        \\centering\n"
        // "        \\noindent\\hrulefill\n"
        // "        \\vspace{0.5cm}\n\n"
        // "        \\begin{minipage}{0.07\\textwidth}\n"
        // "            \\Warning\n"
        // "        \\end{minipage}%\n"
        // "        \\begin{minipage}{0.93\\textwidth}\n"
        // "            Решение, приведенное в данном документе, не претендует на стопроцентную точность, будьте внимательны и аккуратны (а лучше проверяйте решение и репортите ошибки)\n"
        // "        \\end{minipage}\n\n"
        // "        \\vspace{0.4cm}\n"
        // "        \\noindent\\hrulefill\n"
        // "    \\end{figure}\n\n"
    }
};
