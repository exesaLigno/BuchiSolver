#include "split_tree.h"
#include "latex_export.h"
#include "vector_extesions.h"

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <unistd.h>

bool REVERSED_MASK = false;

enum class Operator : uint8_t
{
    TRUE,
    FALSE,
    ATOM,
    UNARY_FIRST,
    NOT = UNARY_FIRST,
    X,
    F,
    G,
    BINARY_FIRST,
    AND = BINARY_FIRST,
    OR,
    IMPL,
    U,
    W,
    R
};

enum class Status : uint8_t
{
    UNKNOWN, TRUE, FALSE
};

using node_ptr = std::shared_ptr<Node<std::vector<Status>>>;

static const struct
{
    char sym;
    Operator opc;
} opcodes[] = {
    {'!', Operator::NOT},
    {'&', Operator::AND},
    {'|', Operator::OR},
    {'U', Operator::U},
    {'F', Operator::F},
    {'G', Operator::G},
    {'R', Operator::R},
    {'W', Operator::W},
    {'X', Operator::X}};

static const std::string DEFINITION_NAMES[] = {
    "\\alpha",
    "\\beta",
    "\\gamma",
    "\\delta",
    "\\varepsilon",
    "\\zeta",
    "\\eta",
    "\\vartheta",
    "\\mu",
    "\\nu",
    "\\xi",
    "\\rho",
    "\\sigma",
    "\\chi",
    "\\psi",
    "\\omega"
};

static char symbol_of(Operator opc)
{
    for (const auto &item : opcodes)
    {
        if (item.opc == opc)
        {
            return item.sym;
        }
    }
    return '\0';
}

static Operator opcode_of(char c)
{
    for (const auto &item : opcodes)
    {
        if (item.sym == c)
        {
            return static_cast<Operator>(item.opc);
        }
    }
    return Operator::FALSE;
}

class Ltl
{
public:
    using Ltl_ptr = std::shared_ptr<Ltl>;

    static Ltl_ptr True()
    {
        static const Ltl_ptr ltl_true = Ltl_ptr(new Ltl(Operator::TRUE));
        return ltl_true;
    }

    static Ltl_ptr False()
    {
        static const Ltl_ptr ltl_false = Ltl_ptr(new Ltl(Operator::FALSE));
        return ltl_false;
    }

    static Ltl_ptr atom(std::string name)
    {
        return Ltl_ptr(new Ltl(std::move(name)));
    }

    static Ltl_ptr unary(Operator opc, const Ltl_ptr &opnd)
    {
        Ltl_ptr ltl = Ltl_ptr(new Ltl(opc));
        ltl->lop = opnd;

        return ltl;
    }

    static Ltl_ptr binary(Operator opc, const Ltl_ptr &lop, const Ltl_ptr &rop)
    {
        Ltl_ptr ltl = Ltl_ptr(new Ltl(opc));
        ltl->lop = lop;
        ltl->rop = rop;

        return ltl;
    }

    Operator kind() const
    {
        return opc;
    }

    const Ltl *lhs() const
    {
        return lop.get();
    }

    const Ltl *rhs() const
    {
        return rop.get();
    }

    void to_string(std::string &s) const
    {
        switch (opc)
        {
            case Operator::TRUE:
                s.append("true");
                break;
            case Operator::FALSE:
                s.append("false");
                break;
            case Operator::ATOM:
                s.append(name);
                break;

            case Operator::IMPL:
                s.push_back('(');
                lop->to_string(s);
                s.append(" -> ");
                rop->to_string(s);
                s.push_back(')');
                break;

            case Operator::NOT:
            case Operator::G:
            case Operator::F:
            case Operator::X:
                s.push_back(symbol_of(opc));
                lop->to_string(s);
                break;

            case Operator::AND:
            case Operator::OR:
            case Operator::U:
            case Operator::R:
            case Operator::W:
                s.push_back('(');
                lop->to_string(s);
                s.push_back(' ');
                s.push_back(symbol_of(opc));
                s.push_back(' ');
                rop->to_string(s);
                s.push_back(')');
                break;
        }
    }

    // returns true if not presented "until" found in current subtree, false if not found 
    bool find_nested_untils(std::vector<const Ltl*>& untils, std::vector<const Ltl*>& currently_found, bool first_call = true) const
    {
        bool already_found = false;

        if (lhs())
            already_found |= lhs()->find_nested_untils(untils, currently_found, false);
        if (rhs())
            already_found |= rhs()->find_nested_untils(untils, currently_found, false);

        if (not first_call and kind() == Operator::U and not already_found and find_if_presented(untils, this) == -1)
        {
            add_if_not_presented(untils, this);
            add_if_not_presented(currently_found, this);
            already_found = true;
        }

        return already_found;
    }

    std::string to_latex_string(std::vector<const Ltl*> definitions = std::vector<const Ltl*>(), std::vector<const Ltl*> just_announced = std::vector<const Ltl*>(), const Ltl* initial_ltl = nullptr) const
    {
        if (initial_ltl and *this == *initial_ltl)
            return "\\varphi";

        std::string s;

        switch (opc)
        {
            case Operator::TRUE:
                s.append("\\TRUE");
                break;
            case Operator::FALSE:
                s.append("\\FALSE");
                break;
            case Operator::ATOM:
                s.append(name);
                break;

            case Operator::IMPL:
                s.push_back('(');
                s.append(lop->to_latex_string(definitions, just_announced));
                s.append(" \\IMPL ");
                s.append(rop->to_latex_string(definitions, just_announced));
                s.push_back(')');
                break;

            case Operator::NOT:
                s.append("\\NOT ");
                s.append(lop->to_latex_string(definitions, just_announced));
                break;

            case Operator::G:
                s.append("\\GLOBALLY ");
                s.append(lop->to_latex_string(definitions, just_announced));
                break;

            case Operator::F:
                s.append("\\FUTURE ");
                s.append(lop->to_latex_string(definitions, just_announced));
                break;

            case Operator::X:
                s.append("\\NEXT ");
                s.append(lop->to_latex_string(definitions, just_announced));
                break;

            case Operator::AND:
                s.push_back('(');
                s.append(lop->to_latex_string(definitions, just_announced));
                s.append(" \\AND ");
                s.append(rop->to_latex_string(definitions, just_announced));
                s.push_back(')');
                break;

            case Operator::OR:
                s.push_back('(');
                s.append(lop->to_latex_string(definitions, just_announced));
                s.append(" \\OR ");
                s.append(rop->to_latex_string(definitions, just_announced));
                s.push_back(')');
                break;

            case Operator::U:
            {
                int definition_idx = find_if_presented(definitions, (const Ltl*) this);
                bool announced = find_if_presented(just_announced, (const Ltl*) this) >= 0;
                if (definition_idx >= 0 and not announced)
                    s.append(DEFINITION_NAMES[definition_idx]);
                else
                {
                    if (definition_idx >= 0 and announced)
                        s.append("\\overbrace{");
                    s.push_back('(');
                    s.append(lop->to_latex_string(definitions, just_announced));
                    s.append(" \\UNTIL ");
                    s.append(rop->to_latex_string(definitions, just_announced));
                    s.push_back(')');
                    if (definition_idx >= 0 and announced)
                    {
                        s.append("}^{");
                        s.append(DEFINITION_NAMES[definition_idx]);
                        s.append("}");
                    }
                }
                break;
            }

            case Operator::R:
                s.push_back('(');
                s.append(lop->to_latex_string(definitions, just_announced));
                s.append(" \\RELEASE ");
                s.append(rop->to_latex_string(definitions, just_announced));
                s.push_back(')');
                break;

            case Operator::W:
                s.push_back('(');
                s.append(lop->to_latex_string(definitions, just_announced));
                s.append(" \\WEAK ");
                s.append(rop->to_latex_string(definitions, just_announced));
                s.push_back(')');
                break;
        }

        return s;
    }

    std::string node_to_string() const
    {
        switch (opc)
        {
            case Operator::TRUE: return "true";
            case Operator::FALSE: return "false";
            case Operator::ATOM: return name;
            case Operator::IMPL: return "{→|implication}";
            case Operator::NOT: return "{!|not}";
            case Operator::G: return "{G|globally}";
            case Operator::F: return "{F|future}";
            case Operator::X: return "{X|next}";
            case Operator::AND: return "{&|and}";
            case Operator::OR: return "{\\||or}";
            case Operator::U: return "{U|until}";
            case Operator::R: return "{R|release}";
            case Operator::W: return "{W|weak until}";
            default: return "{|UNKNOWN|}";
        }
    }

    void dump_to(FILE *f) const
    {
        fprintf(f, "digraph G {\trankdir=LR;\n");
        recursive_dump_to(f);
        fprintf(f, "}");
    }

    bool operator==(const Ltl& other) const
    {
        return (kind() == other.kind() && (kind() != Operator::ATOM || name == other.name)) && 
            ((!lhs() && !other.lhs()) || (lhs() && other.lhs() && *lhs() == *other.lhs())) &&
            ((!rhs() && !other.rhs()) || (rhs() && other.rhs() && *rhs() == *other.rhs()));
    }

    Status calculate(const std::vector<const Ltl*>& all, std::vector<Status>& all_mask) const
    {
        #define SET_AND_RETURN(status) { all_mask[mask_idx] = status; return status; }

        int mask_idx;
        for (mask_idx = 0; mask_idx < all.size(); mask_idx++)
        {
            if (*this == *(all[mask_idx]))
                break;
        }

        auto l_status = lhs() ? lhs()->calculate(all, all_mask) : Status::UNKNOWN;
        auto r_status = rhs() ? rhs()->calculate(all, all_mask) : Status::UNKNOWN;

        if (all_mask[mask_idx] != Status::UNKNOWN)
            return all_mask[mask_idx];

        switch (kind())
        {
            case Operator::TRUE:
                SET_AND_RETURN(Status::TRUE)

            case Operator::FALSE:
                SET_AND_RETURN(Status::FALSE)

            case Operator::NOT:
                switch (l_status)
                {
                    case Status::TRUE: SET_AND_RETURN(Status::FALSE)
                    case Status::FALSE: SET_AND_RETURN(Status::TRUE)
                    default: SET_AND_RETURN(Status::UNKNOWN)
                }

            case Operator::AND:
                if (l_status == Status::TRUE && r_status == Status::TRUE)
                    SET_AND_RETURN(Status::TRUE)
                else if (l_status == Status::FALSE || r_status == Status::FALSE)
                    SET_AND_RETURN(Status::FALSE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            case Operator::OR:
                if (l_status == Status::TRUE || r_status == Status::TRUE)
                    SET_AND_RETURN(Status::TRUE)
                else if (l_status == Status::FALSE && r_status == Status::FALSE)
                    SET_AND_RETURN(Status::FALSE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            case Operator::IMPL:
                if (l_status == Status::FALSE || r_status == Status::TRUE)
                    SET_AND_RETURN(Status::TRUE)
                else if (l_status == Status::TRUE && r_status == Status::FALSE)
                    SET_AND_RETURN(Status::FALSE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            case Operator::U:
                if (r_status == Status::TRUE)
                    SET_AND_RETURN(Status::TRUE)
                else if (l_status == Status::FALSE && r_status == Status::FALSE)
                    SET_AND_RETURN(Status::FALSE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            case Operator::F:
                if (l_status == Status::TRUE)
                    SET_AND_RETURN(Status::TRUE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            case Operator::G:
                if (l_status == Status::FALSE)
                    SET_AND_RETURN(Status::FALSE)
                else
                    SET_AND_RETURN(Status::UNKNOWN)

            // [TODO] INCORRECT
            case Operator::W:
                printf("Can not calc W\n");
                SET_AND_RETURN(Status::UNKNOWN)
            
            // [TODO] INCORRECT
            case Operator::R:
                printf("Can not calc R\n");
                SET_AND_RETURN(Status::UNKNOWN)

            case Operator::ATOM:
            case Operator::X:
            default:
                printf("UNREACHABLE CODE!\n");
                SET_AND_RETURN(Status::UNKNOWN)
        }
    }

    bool introduce_X()
    {
        bool changed = false;

        if (kind() == Operator::X)
        {
            auto arg = lop;

            switch (arg->kind())
            {
                // true or false
                case Operator::TRUE:
                case Operator::FALSE:
                    changed = true;
                    opc = arg->kind();
                    lop = nullptr;
                    rop = nullptr;
                    break;

                // unary operators
                case Operator::NOT:
                case Operator::F:
                case Operator::G:
                    changed = true;
                    opc = arg->kind();
                    arg->opc = Operator::X;
                    break;

                // binary operators
                case Operator::AND:
                case Operator::OR:
                case Operator::IMPL:
                case Operator::U:
                case Operator::W:
                case Operator::R:
                    changed = true;
                    opc = arg->kind();
                    lop = unary(Operator::X, arg->lop);
                    rop = unary(Operator::X, arg->rop);
                    arg->lop = nullptr;
                    arg->rop = nullptr;
                    break;
            }
        }

        if (lhs())
            changed |= lop->introduce_X();
        if (rhs())
            changed |= rop->introduce_X();

        return changed;
    }

    bool substitute_R()
    {
        bool changed = false;

        if (kind() == Operator::R)
        {
            changed = True;
            opc = Operator::NOT;
            lop = binary(Operator::U, unary(Operator::NOT, lop), unary(Operator::NOT, rop));
            rop = nullptr;
        }

        if (lhs())
            changed |= lop->substitute_R();
        if (rhs())
            changed |= rop->substitute_R();
        
        return changed;
    }

    bool substitute_W()
    {
        bool changed = false;

        if (kind() == Operator::W)
        {
            changed = True;
            opc = Operator::OR;
            auto new_lop = binary(Operator::U, lop, rop);
            auto new_rop = unary(Operator::G, lop);
            lop = new_lop;
            rop = new_rop;
        }

        if (lhs())
            changed |= lop->substitute_W();
        if (rhs())
            changed |= rop->substitute_W();
        
        return changed;
    }

    bool substitute_G()
    {
        bool changed = false;

        if (kind() == Operator::G)
        {
            changed = True;
            opc = Operator::NOT;
            lop = unary(Operator::F, unary(Operator::NOT, lop));
        }

        if (lhs())
            changed |= lop->substitute_G();
        if (rhs()) 
            changed |= rop->substitute_G();

        return changed;
    }

    bool substitute_F()
    {
        bool changed = false;

        if (kind() == Operator::F)
        {
            changed = True;
            opc = Operator::U;
            rop = lop;
            lop = True();
        }

        if (lhs())
            changed |= lop->substitute_F();
        if (rhs())
            changed |= rop->substitute_F();

        return changed;
    }

private:
    Ltl(std::string _name)
    {
        nref = 0;
        opc = Operator::ATOM;
        name = std::move(_name);
        lop = nullptr;
        rop = nullptr;
    }

    Ltl(Operator _opc)
    {
        nref = 0;
        opc = _opc;
        lop = nullptr;
        rop = nullptr;
    }

    Ltl(const Ltl &) = delete;
    void operator=(const Ltl &) = delete;

    void recursive_dump_to(FILE *f) const
    {
        fprintf(f, "\taddr%p[label=", this);

        fprintf(f, "\"{%s|{kind = %d}}\"", node_to_string().c_str(), kind());

        fprintf(f, ", shape=\"record\"]\n");

        if (lhs())
        {
            lhs()->recursive_dump_to(f);
            fprintf(f, "\taddr%p -> addr%p[label=\".lhs\"]\n", this, lhs());
        }

        if (rhs())
        {
            rhs()->recursive_dump_to(f);
            fprintf(f, "\taddr%p -> addr%p[label=\".rhs\"]\n", this, rhs());
        }
    }

    int nref;
    Operator opc;
    std::string name;
    Ltl_ptr lop, rop;
};

class Parser
{
    using Ltl_ptr = std::shared_ptr<Ltl>;

    const char *stream;
    std::vector<Ltl_ptr> stack;

public:
    Ltl_ptr parse(const char *s)
    {
        stream = s;
        parse_until('\0');

        assert(stack.size() == 1);
        Ltl_ptr ltl = stack.back();
        stack.pop_back();

        return ltl;
    }

private:
    void parse_until(char endsym)
    {
        char c = *stream;
        while (c && c != endsym)
        {
            parse_term();
            c = *stream;
        }
        assert(c == endsym && "invalid end of stream");

        if (c && c == endsym)
        {
            ++stream;
        }
    }

    void skip_empty()
    {
        const char *s = stream;
        while (*s && isspace(*s))
        {
            ++s;
        }
        stream = s;
    }

    Ltl_ptr parse_atom()
    {
        const char *end = stream;
        while (*end && islower(*end))
        {
            ++end;
        }
        assert(stream < end && "invalid atom token");

        std::string name(stream, end);
        stream = end;

        if (name == "true")
        {
            return Ltl::True();
        }
        if (name == "false")
        {
            return Ltl::False();
        }
        return Ltl::atom(std::move(name));
    }

    void parse_term()
    {
        skip_empty();

        const char c = *stream;
        switch (c)
        {
            default:
                stack.push_back(parse_atom());
                break;

            case '!':
            case 'X':
            case 'F':
            case 'G':
                ++stream;
                parse1(opcode_of(c));
                break;

            case '&':
            case '|':
            case 'U':
            case 'R':
            case 'W':
                ++stream;
                parse2(opcode_of(c));
                break;

            case '-':
                assert(stream[1] == '>' && "invalid token");
                stream += 2;
                parse2(Operator::IMPL);
                break;

            case '(':
                ++stream;
                parse_until(')');
                break;

            case ')':
                break;
        }
    }

    void parse1(Operator opc)
    {
        parse_term();
        Ltl_ptr ltl = pop();
        stack.push_back(Ltl::unary(opc, ltl));
    }

    void parse2(Operator opc)
    {
        parse_term();
        Ltl_ptr rop = pop();
        Ltl_ptr lop = pop();
        stack.push_back(Ltl::binary(opc, lop, rop));
    }

    Ltl_ptr pop()
    {
        Ltl_ptr ltl = stack.back();
        stack.pop_back();
        return ltl;
    }
};

class Automaton
{
    using index_vec_type = std::vector<size_t>;

    std::vector<index_vec_type> adjacent;
    std::vector<index_vec_type> accepting;
    index_vec_type initial;

public:
    Automaton(const Automaton &) = delete;
    Automaton &operator=(const Automaton &) = delete;

    /// Init automaton for a given number of states
    Automaton(size_t card)
    {
        adjacent.resize(card);
    }

    void add_transition(size_t src, size_t dst)
    {
        adjacent[src].push_back(dst);
    }

    void mark_init(size_t state)
    {
        assert(state < adjacent.size() && "invalid state number");
        initial.push_back(state);
    }

    void mark_accept(size_t set, size_t state)
    {
        assert(state < adjacent.size() && "invalid state number");
        if (set >= accepting.size())
        {
            accepting.resize(set + 1);
        }
        accepting[set].push_back(state);
    }

    void finalize()
    {
        for (index_vec_type &values : adjacent)
        {
            deduplicate(values);
        }
        for (index_vec_type &values : accepting)
        {
            deduplicate(values);
        }
        deduplicate(initial);
    }

    void write_to(FILE *f) const
    {
        fprintf(f, "%zu %zu\n", adjacent.size(), accepting.size());
        write_set_to(f, initial);
        for (const index_vec_type &accepting_set : accepting)
        {
            write_set_to(f, accepting_set);
        }

        size_t i = 0;
        for (const index_vec_type &transitions : adjacent)
        {
            write_set_to(f, transitions);
            ++i;
        }
    }

    void write_graph_to(FILE* f) const
    {
        fprintf(f, "digraph G {\n\tgraph[dpi = 400];\n\tlayout=\"circo\";\n\trankdir=TB;\n");
        
        // Creating dummy nodes for initial states
        for (int i = 0; i < initial.size(); i++)
            fprintf(f, "\tn%d[label=\"\",shape=none,height=.0,width=.0]\n", i);

        fprintf(f, "\n");

        // Creating state nodes
        for (int i = 0; i < card(); i++)
        {
            fprintf(f, "\ts%d[shape=\"circle\"", i + 1);

            bool is_accepting = false;
            for (auto ac_vec : accepting)
            {
                for (auto ac : ac_vec)
                {
                    if (ac == i)
                        is_accepting = true;
                }
            }

            if (is_accepting)
                fprintf(f, ", peripheries=2]\n");
            else
                fprintf(f, "]\n");
        }

        fprintf(f, "\n");

        // Adding edges from nowhere to initial nodes
        for (int i = 0; i < initial.size(); i++)
            fprintf(f, "\tn%d->s%d\n", i, initial[i] + 1);

        fprintf(f, "\n");

        // Adding edges between nodes
        for (int i = 0; i < adjacent.size(); i++)
        {
            for (auto j : adjacent[i])
            {
                fprintf(f, "\ts%d->s%d\n", i+1, j+1);
            }
        }

        fprintf(f, "}\n");
    }

    size_t card() const
    {
        return adjacent.size();
    }

private:
    static void write_set_to(FILE *f, const index_vec_type &values)
    {
        fprintf(f, "%zu ", values.size());
        for (size_t v : values)
        {
            fprintf(f, "%zu ", v);
        }
        fputs("\n", f);
    }

    static void deduplicate(index_vec_type &values)
    {
        std::sort(values.begin(), values.end());
        index_vec_type::iterator it =
            std::unique(values.begin(), values.end());
        values.erase(it, values.end());
    }
};

bool iterate_mask(std::vector<bool>& atoms_mask, const int mask_size)
{
    if (atoms_mask.size() == 0)
    {
        for (int i = 0; i < mask_size; i++)
            atoms_mask.push_back(false);

        return true; // We can enter current iteration
    }

    bool IS = REVERSED_MASK; // Iterate from start

    for (int i = IS ? 0 : (mask_size - 1); IS ? (i < mask_size) : (i >= 0); i += IS ? 1 : -1)
    {
        if (!atoms_mask[i])
        {
            atoms_mask[i] = true;

            for (int j = IS ? (i - 1) : (i + 1); IS ? (j >= 0) : (j < mask_size); j += IS ? -1 : 1)
                atoms_mask[j] = false;

            return true; // Found `false` in mask and iterated (010011 -> 010100), so can enter iteration
        }
    }

    return false; // All states already checked, we can't enter iteration
}

void get_atoms(const Ltl* ltl, std::vector<const Ltl*>& atoms)
{
    if (ltl->lhs())
        get_atoms(ltl->lhs(), atoms);

    if (ltl->rhs())
        get_atoms(ltl->rhs(), atoms);

    if (ltl->kind() == Operator::X || ltl->kind() == Operator::ATOM)
        add_if_not_presented(atoms, ltl);
}

void get_all(const Ltl* ltl, std::vector<const Ltl*>& all)
{
    if (ltl->lhs())
        get_all(ltl->lhs(), all);

    if (ltl->rhs())
        get_all(ltl->rhs(), all);

    add_if_not_presented(all, ltl);
}

static std::vector<const Ltl*> transform_ltl(std::shared_ptr<Ltl>& ltl, FILE* output_file = nullptr)
{
    std::vector<const Ltl*> definitions;

    if (output_file)
    {
        fprintf(output_file, "\tПреобразуем исходную формулу\n");
        fprintf(output_file, "\t$$\\varphi = %s", ltl->to_latex_string().c_str());
    }
    while (ltl->introduce_X())
    {
        if (output_file)
            fprintf(output_file, " = \\text{/ Заносим X внутрь операторов /}$$\n\t$$= %s", ltl->to_latex_string().c_str());
    }
    if (ltl->substitute_R() && output_file)
        fprintf(output_file, " = \\text{/ Выражаем R через U /}$$\n\t$$= %s", ltl->to_latex_string().c_str());
    if (ltl->substitute_W() && output_file)
        fprintf(output_file, " = \\text{/ Выражаем W через U и G /}$$\n\t$$= %s", ltl->to_latex_string().c_str());
    if (ltl->substitute_G() && output_file)
        fprintf(output_file, " = \\text{/ Выражаем G через F /}$$\n\t$$= %s", ltl->to_latex_string().c_str());
    if (ltl->substitute_F() && output_file)
        fprintf(output_file, " = \\text{/ Выражаем F через U /}$$\n\t$$= %s", ltl->to_latex_string().c_str());
    if (output_file)
    {
        while (true)
        {
            std::vector<const Ltl*> just_announced;
            bool found = ltl->find_nested_untils(definitions, just_announced);
            if (found)
                fprintf(output_file, " = $$\n\t$$= %s", ltl->to_latex_string(definitions, just_announced).c_str());
            else
            {
                fprintf(output_file, " = $$\n\t$$= %s", ltl->to_latex_string(definitions).c_str());
                break;
            }
        }

        fprintf(output_file, "$$\n");
    }

    return definitions;
}

static node_ptr add_state(const std::shared_ptr<Ltl>& ltl, const std::vector<const Ltl*>& all, std::vector<Status> all_mask, std::vector<std::vector<Status>>& states)
{
    Status result = ltl->calculate(all, all_mask);

    node_ptr current(new Node(all_mask));

    int unknown_until_idx = -1;
    for (int i = 0; i < all.size(); i++)
    {
        if (all_mask[i] == Status::UNKNOWN)
        {
            unknown_until_idx = i;
            break;
        }
    }

    if (unknown_until_idx >= 0)
    {
        all_mask[unknown_until_idx] = Status::FALSE;
        current->set_first(add_state(ltl, all, all_mask, states));
        all_mask[unknown_until_idx] = Status::TRUE;
        current->set_second(add_state(ltl, all, all_mask, states));
    }

    else
        states.push_back(all_mask);

    return current;
}

std::string get_edge_restrictions(const std::vector<const Ltl*>& all, const std::vector<Status>& state, const std::vector<const Ltl *> definitions, const Ltl* initial_ltl)
{
    std::string restrictions;

    for (auto subltl : all)
    {
        switch (subltl->kind())
        {
            case Operator::X:
                if (not restrictions.empty())
                    restrictions.append(" \\AND ");
                restrictions.append(subltl->lhs()->to_latex_string(definitions, std::vector<const Ltl*>(), initial_ltl));
                restrictions.append(state[find_if_presented(all, subltl)] == Status::TRUE ? " \\in " : " \\notin ");
                restrictions.append("s'");
                break;

            case Operator::U:
                if (state[find_if_presented(all, subltl->lhs())] == Status::TRUE &&
                    state[find_if_presented(all, subltl->rhs())] == Status::FALSE)
                {
                    if (not restrictions.empty())
                        restrictions.append(" \\AND ");
                    restrictions.append(subltl->to_latex_string(definitions, std::vector<const Ltl*>(), initial_ltl));
                    restrictions.append(state[find_if_presented(all, subltl)] == Status::TRUE ? " \\in " : " \\notin ");
                    restrictions.append("s'");
                }
                break;
        }
    }

    return restrictions;
}

static bool check_edge_rules(const std::vector<const Ltl*>& all, const std::vector<std::vector<Status>>& states, const int from, const int to)
{
    for (auto a : all)
    {
        if (a->kind() == Operator::U)
        {
            int u_idx = find_if_presented(all, a);
            int u_lhs_idx = find_if_presented(all, a->lhs());
            int u_rhs_idx = find_if_presented(all, a->rhs());

            if (!(
                (states[from][u_idx] == Status::TRUE && states[from][u_rhs_idx] == Status::TRUE) ||  // p U q === true, q === true -> any succesor possible
                (states[from][u_idx] == Status::FALSE && states[from][u_lhs_idx] == Status::FALSE && states[from][u_rhs_idx] == Status::FALSE) ||  // if p U q === false, and p, q === 0 -> any succesor possible
                (states[from][u_lhs_idx] == Status::TRUE && states[from][u_rhs_idx] == Status::FALSE && states[from][u_idx] == states[to][u_idx])  // if p === 1, q === 0 -> successor must have p U q same as prdecessor
            ))
                return false;
        }
        else if (a->kind() == Operator::X)
        {
            int x_idx = find_if_presented(all, a);
            int x_var_idx = find_if_presented(all, a->lhs());

            if (!(
                states[from][x_idx] == states[to][x_var_idx]
            ))
                return false;
        }
    }

    return true;
}

void print_table_line(FILE* dst, const node_ptr& node, const std::vector<const Ltl*>& all, const std::vector<const Ltl *> definitions, const Ltl* initial_ltl, int& states_counter, int columns_count, int column = 0, bool fill_start = false)
{
    if (fill_start)
    {
        for (int i = 0; i < column; i++)
            fprintf(dst, "&");
    }

    std::string truth_list;
    for (int i = 0; i < all.size(); i++)
    {
        if (node->data[i] == Status::TRUE)
        {
            if (not truth_list.empty())
                truth_list.append(", ");

            truth_list.append(all[i]->to_latex_string(definitions, std::vector<const Ltl*>(), initial_ltl));
        }
    }

    if (node->first || node->second)
        fprintf(dst, "\\multirow{%d}{*}{$%s$}", node->leafs_count(), truth_list.c_str());
    else
        fprintf(dst, "\\multirow{%d}{*}{$\\mathbf{s_{%d}}: %s$}", node->leafs_count(), states_counter++, truth_list.c_str());

    if (node->first)
    {
        fprintf(dst, "&");
        print_table_line(dst, node->first, all, definitions, initial_ltl, states_counter, columns_count, column+1);
    }

    else
    {
        for (int i = column; i < columns_count - 1; i++)
            fprintf(dst, "&");
        fprintf(dst, "\\\\\n");
    }

    if (node->second)
        print_table_line(dst, node->second, all, definitions, initial_ltl, states_counter, columns_count, column+1, true);

    fprintf(dst, "\\cline{%d-%d}", column + 1, columns_count);
}

static std::unique_ptr<Automaton> run_ltl_to_buchi(const char *text, FILE* output_file = nullptr)
{
    if (output_file)
        write_preamble(output_file);

    Parser parser;
    std::shared_ptr<Ltl> ltl = parser.parse(text);

    FILE* f = fopen("ltl_before_transform.dot", "w");
    ltl->dump_to(f);
    fclose(f);

    auto definitions = transform_ltl(ltl, output_file);

    f = fopen("ltl_after_transform.dot", "w");
    ltl->dump_to(f);
    fclose(f);

    std::vector<const Ltl*> atoms;
    std::vector<bool> atoms_mask;
    std::vector<const Ltl*> all;
    std::vector<std::vector<Status>> states;

    get_atoms(ltl.get(), atoms);
    get_all(ltl.get(), all);

    if (output_file)
    {
        fprintf(output_file, "\n\tЗапишем таблицу истинности для независимых подформул: ");
        for (int i = 0; i < atoms.size(); i++)
        {
            fprintf(output_file, "$%s$", atoms[i]->to_latex_string().c_str());
            if (i == atoms.size() - 1)
                fprintf(output_file, "\n");
            else
                fprintf(output_file, ", ");
        }
    }

    std::vector<node_ptr> table_states;

    int states_counter = 1;
    while (iterate_mask(atoms_mask, atoms.size()))
    {
        std::vector<Status> all_mask;
        for (auto cur_ltl : all)
        {
            int found = find_if_presented(atoms, cur_ltl);
            if (found >= 0)
                all_mask.push_back(atoms_mask[found] ? Status::TRUE : Status::FALSE);
            else
                all_mask.push_back(Status::UNKNOWN);
        }

        auto split_tree = add_state(ltl, all, all_mask, states);
        table_states.push_back(split_tree);
    }

    if (output_file)
    {
        int max_depth = 0;
        for (auto tree : table_states)
        {
            int depth = tree->depth() + atoms.size();
            if (depth > max_depth)
                max_depth = depth;
        }

        fprintf(output_file, "\t\\begin{table}[h!]\n\t\t\\begin{tabular}{|");
        for (int i = 0; i < atoms.size(); i++)
            fprintf(output_file, "c|");
        for (int i = atoms.size(); i < max_depth; i++)
            fprintf(output_file, "l|");

        fprintf(output_file, "}\n\t\t\t\\hline\n\t\t\t");
        for (int i = 0; i < max_depth; i++)
        {
            if (i < atoms.size())
                fprintf(output_file, "$%s$", atoms[i]->to_latex_string().c_str());
            else
                fprintf(output_file, " ");
            if (i != max_depth - 1)
                fprintf(output_file, "&");
        }
        fprintf(output_file, "\\\\\n\t\t\t\\hline\n");

        std::vector<bool> new_atoms_mask;
        int spilt_tree_idx = 0;
        while (iterate_mask(new_atoms_mask, atoms.size()))
        {
            auto split_tree = table_states[spilt_tree_idx++];

            for (auto atom_state : new_atoms_mask)
                fprintf(output_file, "\\multirow{%d}{*}{%d} & ", split_tree->leafs_count(), atom_state ? 1 : 0);

            print_table_line(output_file, split_tree, all, definitions, ltl.get(), states_counter, max_depth, atoms.size());

            fprintf(output_file, "\\hline\n");
        }

        fprintf(output_file, "\t\t\\end{tabular}\n\t\\end{table}\n");
    }

    std::unique_ptr<Automaton> maton(new Automaton(states.size()));

    if (output_file)
    {
        fprintf(output_file, "\n\tНачальные состояния:\n\n\t$$\n\t\tI = \\{s: \\varphi \\in s\\} = \\{");

        bool first_iter = true;
        int c = 1;
        for (int i = 0; i < states.size(); i++)
        {
            if (states[i].back() == Status::TRUE)
            {
                maton->mark_init(i);
                if (not first_iter)
                    fprintf(output_file, ", ");
                first_iter = false;
                fprintf(output_file, "s_{%d}", i + 1);
            }
        }

        fprintf(output_file, "\\}\n\t$$\n");
    }

    else
    {
        for (int i = 0; i < states.size(); i++)
        {
            if (states[i].back() == Status::TRUE)
                maton->mark_init(i);
        }
    }

    if (output_file)
    {
        int U_count = 0;
        for (auto l : all)
        {
            if (l->kind() == Operator::U)
                U_count++;
        }

        fprintf(output_file, "\n\tВ формуле имеется %d операций $\\UNTIL$, таким образом"
                " будет %d множеств допускающих состояний: \n", 
                U_count, U_count);
    }

    int set_no = 0;
    for (auto l : all)
    {
        if (l->kind() == Operator::U || 
            l->kind() == Operator::F ||
            l->kind() == Operator::G ||
            l->kind() == Operator::R ||
            l->kind() == Operator::W)
        {
            auto right = (l->kind() == Operator::F || l->kind() == Operator::G) ? l->lhs() : l->rhs();

            if (output_file)
                fprintf(output_file, "\n\t$$\n\t\tF_{%s} = \\{s: %s \\in s \\OR %s \\notin s \\} = \\{", 
                        l->to_latex_string(definitions, std::vector<const Ltl*>(), ltl.get()).c_str(), 
                        right->to_latex_string(definitions, std::vector<const Ltl*>(), ltl.get()).c_str(), 
                        l->to_latex_string(definitions, std::vector<const Ltl*>(), ltl.get()).c_str());

            int u_idx = find_if_presented(all, l);
            int u_rhs_idx = find_if_presented(all, right);

            bool first_iter = true;
            for (int i = 0; i < states.size(); i++)
            {
                if (states[i][u_idx] == states[i][u_rhs_idx])
                {
                    maton->mark_accept(set_no, i);

                    if (output_file)
                    {
                        if (!first_iter)
                            fprintf(output_file, ", ");
                        first_iter = false;
                        fprintf(output_file, "s_{%d}", i + 1);
                    }
                }
            }

            if (output_file)
                fprintf(output_file, "\\}\n\t$$\n");

            set_no++;
        }
    }

    if (output_file)
        fprintf(output_file, "\n\tВычислим переходы между узлами:\n\n");

    std::vector<std::string> edge_rules;
    std::vector<std::string> edge_definitions;

    for (int from = 0; from < states.size(); from++)
    {
        if (output_file)
        {
            std::string atoms_truth;
            for (int i = 0; i < atoms.size(); i++)
            {
                int atom_idx = find_if_presented(all, atoms[i]);
                if (atom_idx >= 0 and states[from][atom_idx] == Status::TRUE)
                {
                    if (not atoms_truth.empty())
                        atoms_truth.append(", ");
                    atoms_truth.append(atoms[i]->to_latex_string());
                }
            }

            if (atoms_truth.empty())
                atoms_truth.append("\\varnothing");
            else
                atoms_truth = "\\{" + atoms_truth + "\\}";

            auto rules = get_edge_restrictions(all, states[from], definitions, ltl.get());

            int same_rules_idx = -1;
            for (int i = 0; i < edge_rules.size(); i++)
            {
                if (edge_rules[i] == rules)
                {
                    same_rules_idx = i;
                    break;
                }
            }

            fprintf(output_file, "\t$$\n\t\t\\delta(s_{%d}, %s) = ", from+1, atoms_truth.c_str());

            if (same_rules_idx == -1)
            {
                fprintf(output_file, "\\{s': %s\\} = \\{", rules.c_str());
                edge_rules.push_back(rules);
            }
            else
            {
                fprintf(output_file, "%s = \\{", edge_definitions[same_rules_idx].c_str());
                edge_rules.push_back("");
            }

            edge_definitions.push_back("\\delta(s_{" + std::to_string(from + 1) + "}, " + atoms_truth + ")");
        }

        bool first_iter = true;
        for (int to = 0; to < states.size(); to++)
        {
            if (check_edge_rules(all, states, from, to))
            {
                maton->add_transition(from, to);
                if (output_file)
                {
                    if (not first_iter)
                        fprintf(output_file, ", ");
                    first_iter = false;
                    fprintf(output_file, "s_{%d}", to+1);
                }
            }
        }

        if (output_file)
            fprintf(output_file, "\\}\n\t$$\n");
    }

    FILE* automaton_dump_file = fopen("automaton.dot", "w");
    maton->write_graph_to(automaton_dump_file);
    fclose(automaton_dump_file);

    if (output_file)
        write_ending(output_file);
    
    return maton;
}

int main(int argc, char *argv[])
{
    int ltl_idx = 0;
    int output_file_idx = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-o"))
            output_file_idx = ++i;

        else if (!strcmp(argv[i], "--reverse-mask") || !strcmp(argv[i], "-r"))
            REVERSED_MASK = true;

        else
            ltl_idx = i;
    }

    FILE* output = stdout;
    char* tex_name = nullptr;

    if (output_file_idx != 0)
    {
        tex_name = new char[strlen(argv[output_file_idx]) + 1 + 4];
        strcpy(tex_name, argv[output_file_idx]);

        char* last_dot = strrchr(tex_name, '.');
        if (last_dot == nullptr)
            strcat(tex_name, ".tex");

        else if (!strcmp(last_dot, ".pdf"))
            strcpy(last_dot, ".tex");

        else if (!strcmp(last_dot, ".tex"));

        else
            strcat(tex_name, ".tex");

        output = fopen(tex_name, "w");
    }

    auto buchi = run_ltl_to_buchi(argv[1], output);

    if (output_file_idx != 0)
    {
        fclose(output);

        char* cmd = new char[strlen(tex_name) + sizeof("pdflatex ") + 1];
        strcpy(cmd, "pdflatex ");
        strcat(cmd, tex_name);

        int res = system(cmd);

        if (res == -1)
            printf("Can not find pdflatex, result exported to `%s` but not compiled\n", tex_name);

        else if (res != 0)
            printf("Error occured while compiling `%s`\n", tex_name);

        else
        {
            char* extension = strrchr(tex_name, '.');
            remove(tex_name);
            strcpy(extension, ".aux");
            remove(tex_name);
            strcpy(extension, ".log");
            remove(tex_name);
            strcpy(extension, ".out");
            remove(tex_name);
        }

        delete[] tex_name;
        delete[] cmd;
    }

    return 0;
}
