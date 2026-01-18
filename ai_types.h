// AI Type Enumeration - Shared between main and weighttrainer
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef AI_TYPES_H
#define AI_TYPES_H

enum class AIType {
    SMART_RANDOM,         // Random + win detection (baseline)
    HYBRID_EVALUATOR,     // Tactical + strategic (trainable, strongest)
    HYBRID_EVALUATOR_V2   // Minimax-enhanced version with incremental evaluation
};

#endif // AI_TYPES_H
