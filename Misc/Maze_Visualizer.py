import pygame
import random

# --- Configuration & Constants ---
W, H = 16, 16
CELL_SIZE = 50  
SIDEBAR_WIDTH = 300 
WIDTH, HEIGHT = (W * CELL_SIZE) + SIDEBAR_WIDTH, (H * CELL_SIZE)
FPS = 60

# Colors - Refined Palette
CLR_CANVAS  = (245, 247, 250) 
CLR_SIDEBAR = (22, 24, 28)    
CLR_PANEL   = (34, 37, 43)    
CLR_WALL    = (30, 32, 38)    
CLR_GOAL    = (255, 184, 108) 
CLR_PATH    = (80, 250, 123)  
CLR_ROBOT   = (255, 121, 198) 
CLR_TEXT    = (248, 248, 242) 
CLR_ACCENT  = (139, 233, 253) 
CLR_MUTED   = (98, 114, 164)  

class Cell:
    def __init__(self):
        self.walls = [True, True, True, True] # Top, Right, Bottom, Left
        self.dist = 255
        self.visited = False

class MazeSolver:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((WIDTH, HEIGHT))
        pygame.display.set_caption("Maze Visualizer")
        self.clock = pygame.time.Clock()
        
        font_name = "Segoe UI" if "Segoe UI" in pygame.font.get_fonts() else "Arial"
        self.font_main = pygame.font.SysFont(font_name, 24, bold=True)
        self.font_sub = pygame.font.SysFont(font_name, 15)
        self.font_label = pygame.font.SysFont(font_name, 13, bold=True)
        self.font_dist = pygame.font.SysFont("Consolas", 14, bold=True)
        
        self.speed_level = 3 
        self.step_mode = False 
        self.maze_type = "PERFECT" 
        self.goal_x, self.goal_y = W // 2, H // 2
        self.reset_simulation()

    def reset_simulation(self):
        self.maze = [[Cell() for _ in range(H)] for _ in range(W)]
        self.generate_maze()
        self.reset_solver_state()

    def reset_solver_state(self):
        self.x, self.y = 0, 0
        self.target_x, self.target_y = 0, 0
        self.anim_progress = 1.0
        self.current_dir = 0 
        self.path = [(0, 0)]
        self.running = False
        self.algorithm = "None"
        self.show_flood = False
        self.last_move_time = 0
        self.dfs_stack = [(0, 0)]
        self.dfs_visited = [[False for _ in range(H)] for _ in range(W)]
        self.dfs_visited[0][0] = True
        self.dfs_parent = {}

    def is_void_created(self, x, y, d):
        """Checks if removing wall 'd' at (x,y) creates a 2x2 open area."""
        # Temporary remove to test
        self.maze[x][y].walls[d] = False
        nx, ny = x + (1 if d==1 else -1 if d==3 else 0), y + (1 if d==0 else -1 if d==2 else 0)
        if 0 <= nx < W and 0 <= ny < H:
            self.maze[nx][ny].walls[(d+2)%4] = False

        void = False
        # Check the 4 possible 2x2 squares this wall could be a part of
        for ox in [-1, 0]:
            for oy in [-1, 0]:
                # Check bounds for a 2x2 square starting at x+ox, y+oy
                if 0 <= x+ox < W-1 and 0 <= y+oy < H-1:
                    # A 2x2 is empty if there are no 'inner' walls between the 4 cells
                    # We check the 4 central wall segments of the 2x2 cluster
                    c1, c2, c3, c4 = (x+ox, y+oy), (x+ox+1, y+oy), (x+ox, y+oy+1), (x+ox+1, y+oy+1)
                    if not self.maze[c1[0]][c1[1]].walls[1] and \
                       not self.maze[c1[0]][c1[1]].walls[0] and \
                       not self.maze[c4[0]][c4[1]].walls[3] and \
                       not self.maze[c4[0]][c4[1]].walls[2]:
                        void = True
        
        # Put it back if we are just testing, or leave it if we decide to keep the change elsewhere
        # But for this helper, we revert so the main loop can decide
        self.maze[x][y].walls[d] = True
        if 0 <= nx < W and 0 <= ny < H:
            self.maze[nx][ny].walls[(d+2)%4] = True
        return void

    def generate_maze(self):
        # 1. Generate Perfect Maze
        stack = []
        cx, cy = 0, 0
        self.maze[cx][cy].visited = True
        stack.append((cx, cy))
        while stack:
            cx, cy = stack[-1]
            neighbors = []
            pot = [(cx, cy+1, 0, 2), (cx+1, cy, 1, 3), (cx, cy-1, 2, 0), (cx-1, cy, 3, 1)]
            for nx, ny, w1, w2 in pot:
                if 0 <= nx < W and 0 <= ny < H and not self.maze[nx][ny].visited:
                    neighbors.append((nx, ny, w1, w2))
            if neighbors:
                nx, ny, w1, w2 = random.choice(neighbors)
                self.maze[cx][cy].walls[w1] = self.maze[nx][ny].walls[w2] = False
                self.maze[nx][ny].visited = True
                stack.append((nx, ny))
            else:
                stack.pop()

        # 2. Break walls for Standing Walls (with 2x2 prevention)
        if self.maze_type == "STANDING":
            removal_chance = 0.12 
            for x in range(W):
                for y in range(H):
                    for d in range(2): # Only check Top(0) and Right(1) to avoid double processing
                        if (d == 0 and y == H-1) or (d == 1 and x == W-1): continue
                        
                        if self.maze[x][y].walls[d] and random.random() < removal_chance:
                            if not self.is_void_created(x, y, d):
                                self.maze[x][y].walls[d] = False
                                nx, ny = x + (1 if d==1 else 0), y + (1 if d==0 else 0)
                                self.maze[nx][ny].walls[(d+2)%4] = False

    def get_move_delay(self):
        return [600, 300, 120, 40, 10][self.speed_level - 1]

    def get_anim_speed(self):
        return [0.04, 0.1, 0.25, 0.5, 1.0][self.speed_level - 1]

    def flood_fill(self):
        for i in range(W):
            for j in range(H): self.maze[i][j].dist = 255
        queue = [(self.goal_x, self.goal_y)]
        self.maze[self.goal_x][self.goal_y].dist = 0 
        idx = 0
        while idx < len(queue):
            xq, yq = queue[idx]
            idx += 1
            for d in range(4):
                if not self.maze[xq][yq].walls[d]:
                    nx, ny = xq + (1 if d==1 else -1 if d==3 else 0), yq + (1 if d==0 else -1 if d==2 else 0)
                    if 0 <= nx < W and 0 <= ny < H and self.maze[nx][ny].dist > self.maze[xq][yq].dist + 1:
                        self.maze[nx][ny].dist = self.maze[xq][yq].dist + 1
                        queue.append((nx, ny))

    def solve_step(self):
        if self.x == self.goal_x and self.y == self.goal_y:
            self.running = False
            return
        if self.algorithm == 'Flood Fill':
            self.flood_fill()
            best_dist, best_dir, dx, dy = 255, self.current_dir, 0, 0
            for d in range(4):
                if not self.maze[self.x][self.y].walls[d]:
                    nx, ny = self.x + (1 if d==1 else -1 if d==3 else 0), self.y + (1 if d==0 else -1 if d==2 else 0)
                    if 0 <= nx < W and 0 <= ny < H and self.maze[nx][ny].dist < best_dist:
                        best_dist, best_dir, dx, dy = self.maze[nx][ny].dist, d, nx - self.x, ny - self.y
            self.current_dir = best_dir
            self.set_target(self.x + dx, self.y + dy)
        elif self.algorithm == 'DFS': self.dfs_solve()
        elif self.algorithm == 'Left Hand': self.left_hand_rule()

    def set_target(self, nx, ny):
        self.target_x, self.target_y = nx, ny
        self.anim_progress = 0.0
        self.path.append((nx, ny))

    def left_hand_rule(self):
        dirs = [(self.current_dir + 3) % 4, self.current_dir, (self.current_dir + 1) % 4, (self.current_dir + 2) % 4]
        for d in dirs:
            nx, ny = self.x + (1 if d==1 else -1 if d==3 else 0), self.y + (1 if d==0 else -1 if d==2 else 0)
            if 0 <= nx < W and 0 <= ny < H and not self.maze[self.x][self.y].walls[d]:
                self.current_dir = d
                self.set_target(nx, ny)
                return
        self.running = False

    def dfs_solve(self):
        if not self.dfs_stack: return
        cx, cy = self.dfs_stack[-1]
        for d in range(4):
            nx, ny = cx + (1 if d==1 else -1 if d==3 else 0), cy + (1 if d==0 else -1 if d==2 else 0)
            if not self.maze[cx][cy].walls[d] and 0 <= nx < W and 0 <= ny < H and not self.dfs_visited[nx][ny]:
                self.dfs_visited[nx][ny] = True
                self.dfs_stack.append((nx, ny))
                self.dfs_parent[(nx, ny)] = (cx, cy)
                self.current_dir = d
                self.set_target(nx, ny)
                return
        self.dfs_stack.pop()
        if (cx, cy) in self.dfs_parent:
            px, py = self.dfs_parent[(cx, cy)]
            self.set_target(px, py)

    def draw_sidebar(self):
        sb_x = W * CELL_SIZE
        pygame.draw.rect(self.screen, CLR_SIDEBAR, (sb_x, 0, SIDEBAR_WIDTH, HEIGHT))
        title = self.font_main.render("MAZE VISUALIZER", True, CLR_TEXT)
        self.screen.blit(title, (sb_x + 25, 30))
        pygame.draw.line(self.screen, CLR_PANEL, (sb_x + 25, 70), (sb_x + SIDEBAR_WIDTH - 25, 70), 2)
        
        card_y = 90
        pygame.draw.rect(self.screen, CLR_PANEL, (sb_x + 20, card_y, SIDEBAR_WIDTH - 40, 150), border_radius=10)
        items = [("Algorithm", self.algorithm, CLR_ACCENT), ("Type", self.maze_type, CLR_PATH), ("Status", "RUNNING" if self.running else "IDLE", CLR_ROBOT), ("Mode", "STEP" if self.step_mode else "AUTO", CLR_MUTED)]
        for i, (label, val, color) in enumerate(items):
            self.screen.blit(self.font_label.render(label.upper(), True, CLR_MUTED), (sb_x + 40, card_y + 20 + (i * 32)))
            txt = self.font_sub.render(val, True, color)
            self.screen.blit(txt, (sb_x + SIDEBAR_WIDTH - 40 - txt.get_width(), card_y + 18 + (i * 32)))

        ctrl_y = 265
        controls = [("T", "Toggle Type"), ("R", "Regenerate"), ("F", "Flood Fill"), ("D", "DFS"), ("L", "Left Hand"), ("SPC", "Next"), ("M", "Mode")]
        for i, (key, desc) in enumerate(controls):
            row_y = ctrl_y + 25 + (i * 45)
            pygame.draw.rect(self.screen, CLR_PANEL, (sb_x + 20, row_y, SIDEBAR_WIDTH - 40, 38), border_radius=6)
            k_txt = self.font_label.render(key, True, CLR_ACCENT)
            self.screen.blit(k_txt, (sb_x + 35, row_y + 10))
            self.screen.blit(self.font_sub.render(desc, True, CLR_TEXT), (sb_x + 75, row_y + 10))

    def draw_maze(self):
        if self.show_flood: self.flood_fill()
        for i in range(W):
            for j in range(H):
                px, py = i * CELL_SIZE, (H - j - 1) * CELL_SIZE
                if self.show_flood and self.maze[i][j].dist < 255:
                    txt = self.font_dist.render(str(self.maze[i][j].dist), True, CLR_MUTED)
                    self.screen.blit(txt, txt.get_rect(center=(px + CELL_SIZE//2, py + CELL_SIZE//2)))
                pts = [(px, py), (px+CELL_SIZE, py), (px+CELL_SIZE, py+CELL_SIZE), (px, py+CELL_SIZE)]
                for d in range(4):
                    if self.maze[i][j].walls[d]:
                        pygame.draw.line(self.screen, CLR_WALL, pts[d], pts[(d+1)%4], 5)

    def run(self):
        while True:
            self.screen.fill(CLR_CANVAS)
            for event in pygame.event.get():
                if event.type == pygame.QUIT: pygame.quit(); return
                if event.type == pygame.MOUSEBUTTONDOWN:
                    mx, my = event.pos
                    if mx < W * CELL_SIZE: self.goal_x, self.goal_y = mx // CELL_SIZE, H - 1 - (my // CELL_SIZE)
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_r: self.reset_simulation()
                    if event.key == pygame.K_t: 
                        self.maze_type = "STANDING" if self.maze_type == "PERFECT" else "PERFECT"
                        self.reset_simulation()
                    if event.key == pygame.K_f: self.reset_solver_state(); self.algorithm = 'Flood Fill'; self.running = True
                    if event.key == pygame.K_d: self.reset_solver_state(); self.algorithm = 'DFS'; self.running = True
                    if event.key == pygame.K_l: self.reset_solver_state(); self.algorithm = 'Left Hand'; self.running = True
                    if event.key == pygame.K_s: self.show_flood = not self.show_flood
                    if event.key == pygame.K_m: self.step_mode = not self.step_mode
                    if event.key == pygame.K_UP: self.speed_level = min(5, self.speed_level + 1)
                    if event.key == pygame.K_DOWN: self.speed_level = max(1, self.speed_level - 1)
                    if event.key == pygame.K_SPACE and self.running and self.anim_progress >= 1.0: self.solve_step()

            self.draw_maze()
            gx, gy = self.goal_x * CELL_SIZE, (H - self.goal_y - 1) * CELL_SIZE
            pygame.draw.rect(self.screen, CLR_GOAL, (gx+15, gy+15, CELL_SIZE-30, CELL_SIZE-30), border_radius=4)
            if len(self.path) > 1:
                pts = [(p[0]*CELL_SIZE + CELL_SIZE//2, (H-p[1]-1)*CELL_SIZE + CELL_SIZE//2) for p in self.path]
                pygame.draw.lines(self.screen, CLR_PATH, False, pts, 4)
            
            rx = (self.x + (self.target_x - self.x) * self.anim_progress) * CELL_SIZE + CELL_SIZE // 2
            ry = (H - (self.y + (self.target_y - self.y) * self.anim_progress) - 1) * CELL_SIZE + CELL_SIZE // 2
            pygame.draw.circle(self.screen, CLR_ROBOT, (int(rx), int(ry)), CELL_SIZE // 4)
            self.draw_sidebar()
            
            if self.anim_progress < 1.0:
                self.anim_progress += self.get_anim_speed()
                if self.anim_progress >= 1.0: self.x, self.y = self.target_x, self.target_y
            elif self.running and not self.step_mode and pygame.time.get_ticks() - self.last_move_time > self.get_move_delay():
                self.solve_step()
                self.last_move_time = pygame.time.get_ticks()

            pygame.display.flip()
            self.clock.tick(FPS)

if __name__ == "__main__":
    MazeSolver().run()
