# InfiniTTT - Infinite Tic-Tac-Toe

An advanced implementation of Tic-Tac-Toe on an infinite board with AI opponents that can **learn from experience** through self-play evolution.

## Features

### 🎮 Game Mechanics
- **Infinite Board**: Play on an unlimited 2D coordinate grid
- **5-in-a-Row**: Win by aligning 5 marks (not just 3)
- **Sparse Storage**: Efficient memory usage - only occupied cells are stored
- **Multiple Play Modes**: Human vs Human, Human vs AI, AI vs AI

### 🤖 Advanced AI
- **Hybrid Evaluator AI**: Combines tactical win detection with strategic position evaluation
- **Smart Random AI**: Random play with win/block detection (baseline)
- **Trainable Weights**: Hybrid Evaluator learns optimal evaluation parameters
- **Smart Move Generation**: Only considers positions adjacent to existing pieces

### 🧠 Machine Learning System
- **Self-Play Learning**: AI improves through playing against itself
- **Genetic Algorithm**: Evolves optimal evaluation weights
- **Gapped Sequence Recognition**: Detects patterns like `XX_X` (4-in-a-row with gap)
- **Position Evaluation**: Counts potential winning sequences in 5-cell windows
- **Double Threat Detection**: Recognizes when multiple winning moves exist simultaneously
- **Weight Optimization**: Learns optimal values for different patterns

### 📊 Training Features
- Round-robin tournaments between AI configurations
- Elitism (top 20% survive each generation)
- Crossover and mutation for evolution
- Configurable population size and generations
- Weight persistence (save/load best configurations)

## Building

### Requirements
- C++20 compatible compiler (GCC 13+, Clang 15+)
- CMake 3.0 or higher

### Compilation
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Interactive Play
```bash
./InfiniTTT
```
Choose player types:
1. Human
2. Smart Random AI
3. Hybrid Evaluator AI

### AI Training Mode
Train the Hybrid Evaluator AI to discover optimal strategies:

```bash
# Basic training (10 generations, 20 candidates, 6 games each)
./InfiniTTT --train

# Custom training
./InfiniTTT --train [generations] [population] [games_per_matchup]

# Examples:
./InfiniTTT --train 5 10 4      # Quick test
./InfiniTTT --train 20 30 10    # Serious training
```

**Training Parameters:**
- `generations`: Number of evolution cycles (default: 10)
- `population`: Number of weight candidates (default: 20)
- `games_per_matchup`: Games per pair (default: 6)
  - Higher = more stable results but slower
  - Recommended: 6-10 for good balance

**Estimated Time:**
- 5 generations, 10 candidates: ~10 minutes
- 10 generations, 20 candidates: ~1 hour
- 20 generations, 30 candidates: ~4 hours

### Benchmark Mode
Compare AI performance:
```bash
./InfiniTTT --benchmark [num_games]
./InfiniTTT --benchmark --all [num_games]
```

### Using Trained Weights
```bash
./InfiniTTT --use-trained-weights
./InfiniTTT --benchmark --use-trained-weights --all 50
```

### Help
```bash
./InfiniTTT --help
```

## How the AI Works

### Position Evaluation
The AI evaluates board positions by counting potential winning sequences in **5-cell windows**:

**Patterns Detected:**
- `XXXX_` or `XX_XX` → 4 pieces in window (one move from winning!)
- `XXX__` or `XX_X_` → 3 pieces in window (building position)
- `X___X` → 2 pieces in window (early setup)

**Scoring (configurable through learning):**
- 4-in-a-row, both ends open: **565 points** (can win multiple ways)
- 4-in-a-row, one end blocked: **211 points** (still dangerous)
- 3-in-a-row, both ends open: **40 points** (good potential)
- 3-in-a-row, one end blocked: **19 points** (limited)
- 2-in-a-row, both ends open: **7 points** (positioning)
- Double threat (2+ winning moves): **10000 points** (opponent cannot defend)

**Key Innovation: Gapped Sequence Recognition**

The AI recognizes patterns with gaps, like:
- `XX_X` (one move creates `XXXX`)
- `X_XX` (same strategic value)
- `XXX_X` (one move wins!)

This makes the AI significantly stronger than naive implementations.

### Learning System

**Genetic Algorithm Process:**

1. **Population Creation**: Generate variations of weight configurations
   ```
   Initial: {565, 211, 40, 19, 7, 10000}
   Mutated: {520, 185, 55, 18, 6, 12500}
   ```

2. **Tournament**: Each AI plays every other AI (round-robin)
   - 20 candidates = 190 matchups
   - 6 games per matchup = 1,140 games per generation

3. **Fitness Evaluation**: Win rate = (wins + 0.5 × draws) / total games

4. **Evolution**:
   - Keep top 20% (elitism)
   - Breed new candidates via crossover
   - Mutate 15% for exploration

5. **Iteration**: Repeat for multiple generations

**Result**: AI discovers optimal strategies beyond hand-tuned heuristics!

## Architecture

### Core Components

**TicTacToeBoard** (`tictactoeboard.h/cpp`)
- Sparse board representation using `std::map`
- Win detection in all 4 directions
- Position evaluation with configurable weights

**AIPlayer** (`src/ai/aiplayer.h`)
- Abstract base class for AI implementations
- `HybridEvaluatorAI`: Combines tactical and strategic play (trainable)
- `SmartRandomAI`: Random play with win/block detection (baseline)

**EvaluationWeights** (`evaluationweights.h`)
- Configurable scoring parameters
- Mutation and crossover operations
- File persistence (save/load)

**WeightTrainer** (`weighttrainer.h/cpp`)
- Genetic algorithm implementation
- Tournament system
- Population evolution

### Data Structures

**Board Storage:**
```cpp
std::map<std::pair<int, int>, char> board;
// Sparse: only stores occupied positions
// Infinite: supports any integer coordinates
```

**Sequence Detection:**
```cpp
// Check all 5-cell windows in 4 directions
for each occupied position:
  for each direction (H, V, D\, D/):
    for each offset (0-4):
      analyze 5-cell window
      count friendly/opponent/empty cells
      score based on pattern
```

## Configuration Files

### hybrid_evaluator_weights.txt
Stores the learned weights for Hybrid Evaluator AI:
```
565
211
40
19
7
10000
```

The AI automatically loads this file when using `--use-trained-weights`, allowing continuous improvement across multiple training sessions.

## Examples

### Example Training Session
```
$ ./InfiniTTT --train 5 8 4

=== AI Weight Training Mode ===
Training: Hybrid Evaluator

Configuration:
  Generations: 5
  Population size: 8
  Games per matchup: 4
  Estimated total games: 560

Using default weights as starting point
Evaluation Weights:
  4-open: 565
  4-blocked: 211
  3-open: 40
  3-blocked: 19
  2-open: 7
  double-threat: 10000

Generation 1/5:
  Running tournament.......
  Best candidate:
    Fitness: 0.714 (W:20 L:8 D:0)
    Evaluation Weights:
      4-open: 580
      4-blocked: 205
      3-open: 38
      3-blocked: 21
      2-open: 7
      double-threat: 10500

[... continues for 5 generations ...]

Best weights saved to hybrid_evaluator_weights.txt

Use with: ./InfiniTTT --use-trained-weights
```

### Example Game
```
$ ./InfiniTTT

Welcome to Infinite Tic-Tac-Toe!

Player 1 (X) type:
1. Human
2. Smart Random AI
3. Hybrid Evaluator AI
Enter choice (1-3): 3

Player 2 (O) type:
1. Human
2. Smart Random AI
3. Hybrid Evaluator AI
Enter choice (1-3): 2

Game Configuration:
Player 1 (X): AI (Hybrid Evaluator)
Player 2 (O): AI (Smart Random)

Player X (AI - Hybrid Evaluator) is making a move...
AI played at (0, 0)
 X  0
 0

Player O (AI - Smart Random) is making a move...
AI played at (1, 0)
 X O  0
 0 1

[... game continues ...]
```

## Performance Notes

### Hybrid Evaluator AI
- **Three-level priority system**:
  1. Take winning moves immediately
  2. Block opponent winning moves
  3. Maximize position evaluation score
- **Fast evaluation**: ~1000 positions/second
- **Smart move generation**: Only considers adjacent cells

### Training Performance
- **Computation**: O(n²) where n = population size
- **Per generation**: (n × (n-1) / 2) × games × moves
- **Speed**: ~10-20 games/second (depends on game length)

**Optimization Tips:**
- Start with small populations (10-15) for quick feedback
- Use 4-6 games per matchup for development
- Run serious training (10+ games) overnight

## Advanced Usage

### Custom Weight Files
Create your own starting weights:
```bash
echo "600" > hybrid_evaluator_weights.txt   # four_open
echo "250" >> hybrid_evaluator_weights.txt  # four_blocked
echo "60" >> hybrid_evaluator_weights.txt   # three_open
echo "25" >> hybrid_evaluator_weights.txt   # three_blocked
echo "7" >> hybrid_evaluator_weights.txt    # two_open
echo "10000" >> hybrid_evaluator_weights.txt # double_threat
```

### Benchmark Comparisons
```bash
# Compare two AI types interactively
./InfiniTTT --benchmark 100

# Test all matchups (2x2 grid = 4 total matchups)
./InfiniTTT --benchmark --all 50

# Benchmark with trained weights
./InfiniTTT --benchmark --use-trained-weights --all 50
```

## Technical Details

### Win Condition
- Requires **5 consecutive marks** in any direction:
  - Horizontal (→)
  - Vertical (↑)
  - Diagonal ascending (↗)
  - Diagonal descending (↘)

### Move Generation
- Only considers positions adjacent (8-directional) to existing marks
- First move always at origin (0, 0)
- Prevents exponential search space on infinite board

### AI Strategy
- **Priority-based decision making**: Win > Block > Evaluate
- **Window-based pattern recognition**: Analyzes 5-cell sequences
- **Double threat detection**: Identifies multiple simultaneous winning positions
- **Efficient move ordering**: Only evaluates adjacent positions

## Contributing

This project demonstrates:
- Game tree search algorithms (minimax, alpha-beta)
- Machine learning (genetic algorithms)
- Evolutionary computation
- C++ best practices (smart pointers, const correctness)
- Sparse data structures

Potential improvements:
- Neural network evaluation (AlphaZero-style)
- Monte Carlo Tree Search (MCTS)
- Opening book construction
- Parallel tournament evaluation
- Advanced threat space search
- Multi-objective optimization (speed vs accuracy)

## License

GPL-3.0-only

## Author

Ran Rutenberg <ran.rutenberg@gmail.com>

---

**Have fun and may the best AI win!** 🎮🤖
