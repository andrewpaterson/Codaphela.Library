# Compiler Design

How the compiler is designed and why, for whoever changes it: the decisions behind each part, with their reasons and
dates, and what isn't done.  [Inside the Compiler](../inside.md) and the pages beside it describe what the compiler
does; these pages don't repeat them.

| Page | Is |
|---|---|
| [Front End](front-end.md) | The preprocessor, parser, declarations, types, constant expressions, exceptions and C++14 compliance. |
| [IR and Optimiser](ir.md) | Why there is an IR, the IR front end, the optimiser's passes and their order, inlining and profiles. |
| [W65C816 Back End](w65c816.md) | The target machine, the large model, D as the frame, calls, widths, alignment, allocation and speed. |
| [65C64 Back End](65c64.md) | The back end's shape, its registers (V, W and B as homes), its passes and the choices it makes. |
| [Self-Hosting](self-hosting.md) | Compiling the compiler for the W65C816 and running it there: where that stands and what blocks it. |

The plans the compiler is still being built from stay beside its code: [`CppCompilerLib/Cpp14CompliancePlan.txt`],
[`CppCompilerLib/W65C816.txt`], [`CppCompilerLib/SemanticLayerPlan.txt`] and `ConsolidationPlan.txt`.
