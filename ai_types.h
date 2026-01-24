// AI Type Enumeration - Shared between main and weighttrainer
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

enum class AIType {
    SMART_RANDOM,         // Random + win detection (baseline)
    HYBRID_EVALUATOR,     // Tactical + strategic (trainable, strongest)
    HYBRID_EVALUATOR_V2   // Minimax-enhanced version with incremental evaluation
};
