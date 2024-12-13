#include <cstdio>
#include <cstdlib>

void write_preamble(FILE* dst)
{
    fprintf(dst, "\\documentclass[a4paper, 11pt]{article}\n"
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
