# InfiniTTT - Infinite Tic-Tac-Toe

An advanced implementation of Tic-Tac-Toe on an infinite board with AI opponents that can **learn from experience** through self-play evolution.

## Features

### 🎮 Game Mechanics
- **Infinite Board**: Play on an unlimited 2D coordinate grid
- **5-in-a-Row**: Win by aligning 5 marks (not just 3)
- **Sparse Storage**: Efficient memory usage - only occupied cells are stored
- **Multiple Play Modes**: Human vs Human, Human vs AI, AI vs AI

### 🤖 Advanced AI
- **Minimax with Alpha-Beta Pruning**: Strategic AI that plans ahead
- **Depth-Limited Search**: Configurable search depth (default: 3 levels)
- **Time-Limited Computation**: 100ms thinking time per move
- **Smart Move Generation**: Only considers positions adjacent to existing pieces

### 🧠 Machine Learning System
- **Self-Play Learning**: AI improves through playing against itself
- **Genetic Algorithm**: Evolves optimal evaluation weights
- **Gapped Sequence Recognition**: Detects patterns like `XX_X` (4-in-a-row with gap)
- **Position Evaluation**: Counts potential winning sequences (3+, 4+, 5)
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
./tictactoe
```
Choose player types:
1. Human
2. AI (Minimax)
3. AI (Random)

### AI Training Mode
Train the AI to discover optimal strategies:

```bash
# Basic training (10 generations, 20 candidates, 6 games each)
./tictactoe --train

# Custom training
./tictactoe --train [generations] [population] [games_per_matchup]

# Examples:
./tictactoe --train 5 10 4      # Quick test
./tictactoe --train 20 30 10    # Serious training
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
./tictactoe --benchmark [num_games]
./tictactoe --benchmark --all [num_games]
```

### Help
```bash
./tictactoe --help
```

## How the AI Works

### Position Evaluation
The AI evaluates board positions by counting potential winning sequences in **5-cell windows**:

**Patterns Detected:**
- `XXXX_` or `XX_XX` → 4 pieces in window (one move from winning!)
- `XXX__` or `XX_X_` → 3 pieces in window (building position)
- `X___X` → 2 pieces in window (early setup)

**Scoring (configurable through learning):**
- 4-in-a-row, both ends open: **500 points** (can win multiple ways)
- 4-in-a-row, one end blocked: **200 points** (still dangerous)
- 3-in-a-row, both ends open: **50 points** (good potential)
- 3-in-a-row, one end blocked: **20 points** (limited)
- 2-in-a-row, both ends open: **5 points** (positioning)

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
   Initial: {500, 200, 50, 20, 5}
   Mutated: {520, 185, 55, 18, 6}
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

**AIPlayer** (`aiplayer.h/cpp`)
- Abstract base class for AI implementations
- `MinimaxAI`: Strategic planning with alpha-beta pruning
- `RandomAI`: Baseline random player

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

### best_weights.txt
Stores the best learned weights:
```
500
200
50
20
5
```

The AI automatically loads this file if it exists, allowing continuous improvement across multiple training sessions.

## Examples

### Example Training Session
```
$ ./tictactoe --train 5 8 4

=== AI Weight Training Mode ===

Configuration:
  Generations: 5
  Population size: 8
  Games per matchup: 4
  Estimated total games: 560

Using default weights as starting point
Evaluation Weights:
  4-open: 500
  4-blocked: 200
  3-open: 50
  3-blocked: 20
  2-open: 5

Generation 1/5:
  Running tournament.......
  Best candidate:
    Fitness: 0.714 (W:20 L:8 D:0)
    Evaluation Weights:
      4-open: 520
      4-blocked: 195
      3-open: 48
      3-blocked: 22
      2-open: 5

[... continues for 5 generations ...]

Best weights saved to best_weights.txt
```

### Example Game
```
$ ./tictactoe

Player 1 (X) - Select type:
1. Human
2. AI (Minimax)
3. AI (Random)
Enter choice (1-3): 2

Player 2 (O) - Select type:
1. Human
2. AI (Minimax)
3. AI (Random)
Enter choice (1-3): 2

Player X (Minimax) is making a move...
AI played at (0, 0)
 X  0
 0

Player O (Minimax) is making a move...
AI played at (1, 0)
 X O  0
 0 1

[... game continues ...]
```

## Performance Notes

### Minimax Search
- **Depth 3**: Evaluates ~1,000 positions per move
- **Alpha-beta pruning**: Reduces nodes by ~50-90%
- **Time limit**: 100ms ensures responsive gameplay

### Training Performance
- **Computation**: O(n²) where n = population size
- **Per generation**: (n × (n-1) / 2) × games × moves
- **Bottleneck**: Minimax depth-3 search

**Optimization Tips:**
- Start with small populations (10-15) for quick feedback
- Use 4-6 games per matchup for development
- Run serious training (10+ games) overnight

## Advanced Usage

### Custom Weight Files
Create your own starting weights:
```bash
echo "600" > my_weights.txt  # four_open
echo "250" >> my_weights.txt # four_blocked
echo "60" >> my_weights.txt  # three_open
echo "25" >> my_weights.txt  # three_blocked
echo "7" >> my_weights.txt   # two_open

# Rename to best_weights.txt or modify code
```

### Benchmark Comparisons
```bash
# Compare trained AI vs default
./tictactoe --benchmark 100

# Test all matchups
./tictactoe --benchmark --all 50
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

### Alpha-Beta Pruning
- Prunes ~60-80% of search tree
- Best move ordering improves pruning effectiveness
- Maintains optimal minimax result

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
- Advanced pattern recognition

## License

GPL-3.0-only

## Author

Ran Rutenberg <ran.rutenberg@gmail.com>

---

**Have fun and may the best AI win!** 🎮🤖
