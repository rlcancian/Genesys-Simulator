---
document_type: validation-evidence
authority: informative
owner: continuous-models-implementation
created: 2026-10-30
status: active
tracks: 550
---

# PR #550 — Hosted CI closeout for concurrent-entity regressions

## Scope

Record the hosted validation of the ODE concurrent-arrival test added in
source/test commit `42bb445f2b63830e5622beec0377be4e04598502`. The tested change
was included in documentation/validation commit
`f8f101f38a400726a745c3700859c0585ac133a2`; no production source was changed
between those commits.

## Result

**Confirmed by execution:** all 16 checks reported by `gh pr checks 550` passed
for PR head `f8f101f38a400726a745c3700859c0585ac133a2` on 2026-10-30. The
associated PR merge-ref commit at query time was
`2e2b427b15b272b2570f34008337193085b7b739`.

The checks were:

| Gate | Result | Run / job |
|---|---|---|
| Unit tests | Pass; 1,942 passed, 4 disabled | [41067501633 / 124102673434](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501633/job/124102673434) |
| Kernel unit tests | Pass; 1,942 passed, 4 disabled | [41067501837 / 124102673380](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501837/job/124102673380) |
| Smoke tests | Pass; 3/3 | [41067501837 / 124102672926](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501837/job/124102672926) |
| GUI GMDD diagnostics | Pass | [41067501633 / 124102673000](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501633/job/124102673000) |
| GUI startup: Worker | Pass | [41067502258 / 124102673983](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067502258/job/124102673983) |
| GUI startup: Shell | Pass | [41067501663 / 124102671816](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501663/job/124102671816) |
| GUI startup: AI Assistant | Pass | [41067501734 / 124102672396](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501734/job/124102672396) |
| GUI startup: Data Analyser | Pass | [41067501630 / 124102672164](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501630/job/124102672164) |
| GUI startup: Optimizer | Pass | [41067501674 / 124102671935](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501674/job/124102671935) |
| Debian package build | Pass | [41067501629 / 124103090399](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501629/job/124103090399) |
| Debian package lifecycle | Pass | [41067501629 / 124113025965](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501629/job/124113025965) |
| Plugin archives/link graph (absent) | Pass | [41067501638 / 124102672328](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501638/job/124102672328) |
| Plugin archives/link graph (present) | Pass | [41067501638 / 124102671656](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501638/job/124102671656) |
| Generated plugin target graph | Pass | [41067502171 / 124102674336](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067502171/job/124102674336) |
| AI documentation governance | Pass | [41067501744 / 124102672507](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501744/job/124102672507) |
| Launcher | Pass | [41067501623 / 124102671061](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41067501623/job/124102671061) |

## Bounded conclusion

Hosted CI confirms these automated gates on the recorded validation head. It
does not provide independent technical review, prove manual interaction with
the GUI, or close the broader cancellation/destruction/persistence matrix. The
`HUM-SCI-002` question about invalidating already calculated work when an
ordinary discrete event changes an input remains open. PR #550 remains open and
draft; `reviewDecision` was empty at the final query for this checkpoint.

The GUI results are startup and diagnostic checks, not a manual reproduction of
the original pause/resume GUI crash or a functional continuous-model editor
workflow.
