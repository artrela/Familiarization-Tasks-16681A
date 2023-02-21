# Task 7.1:  Programming Familiarization Part 1 - C++
<br>

![video-game](https://github.com/artrela/Familiarization-Tasks-16681A/blob/master/Task%207-1%20-%20Programming%20Familiarization%20Part%201%20Cpp/output_example.gif)

*Purpose: To give you practical experience in writing and debugging actual C++ applications and give you
more practice at memory in C++ ~ instituted a controller on a pre-existing glfw based video game (see above* 

<hr>

## Q1: AI Mechanics


### *Q. Write an overview of how your AI works, including how it detects where projectiles will fall and how it chooses where to go.*

#### Overview:
Within the game class we have access to: explosions, projectiles, the player, as well as any relevant game parameters. At a high level, the AI performs the following tasks: <br>
1. **Projectile Tracking:** Predict land time and land position of current projectiles.
2. **Determine Safe Spots (1):** If the projectile is landing within a certain amount of time, then mark that area as unsafe. 
3. **Determine Safe Spots (2):** Also mark the current explosion areas as unsafe.
4. **Pick a Safe Spot:** Have the player either go to the left or right depending on the what unsafe areas are near the player. 

#### Part 1. Projectile Tracking

**Purpose:** Predict land time and land position of current projectiles.

**Steps:**
1. Using the equation [ y = y0 +  vy0t + 0.5at^2; y = 0], solve for t using the quatratic equation.
2. Only take this a positive time.
3. Use this time to figure out where the projectile will land with the equation [ x = x0 + vx * t ]

```cpp
Prediction Controller::trackProjectile(const Projectile& p, const Game& g)
	{
		Prediction pred;

		//  -> make vy = 0
		// 1. 
		float t_plus = (- p.vy + std::sqrt(p.vy*p.vy - (4 * (0.5 * g.getGravity()) * p.y)) ) / (2 * (0.5 * g.getGravity()));
		float t_minus = (- p.vy - std::sqrt(p.vy*p.vy - (4 * (0.5 * g.getGravity()) * p.y)) ) / (2 * (0.5 * g.getGravity()));
		// 2. 
		if(t_plus >= 0){pred.t = t_plus;}
		else{pred.t = t_minus;}
		// 3.
		pred.x = p.x + p.vx * pred.t;

		return pred;
	}
```

#### Part 2. Determine Safe Spots (1)

**Purpose:** If the projectile is landing within a certain amount of time, then mark that area as unsafe.

**Steps:**
1. Create an array that pans the width of the screen. Ones are unsafe.
2. Iterate through all projectiles, and predict their landing position & time.
3. Use the iterator to obtain the current projectile's prediction.
4. If the landing time is within 10 seconds, update put unsafe spots in safe spots. This ten seconds was a tuned parameter based on observed results.
5. If the projectile will land in 10 seconds, mark an unsafe zone where it will lands, a width of 1.5 the explosion size. This buffer was added to improve performance.
6. Clip the explosions left and right, to ensure the unsafe spots are within the bounds of the screen.
7. Based on the window that defines the future explosion, update the list of safe spots.
8. Also need to remove 

```cpp
std::vector<bool> Controller::determineSafeSpots(const Game& g)
	{
		// 1.
		std::vector<bool> unsafe_spots(g.getWidth(),0);

		// 2.
		std::list<Projectile> projectile_list = g.getProjectiles();

		std::list<Projectile>::iterator projectile;
		for(projectile = projectile_list.begin(); projectile != projectile_list.end(); projectile++){
			// 3. 
			Prediction pred = trackProjectile(*projectile, g);

			// 4. 
			if(pred.t < 10 && pred.x <= g.getWidth() && pred.x >= 0){

				// 5. 
				float unsafe_right = pred.x + g.explosionSize * 1.5;
				float unsafe_left = pred.x - g.explosionSize * 1.5;

				// 6. 
				if(unsafe_right > g.getWidth()){unsafe_right = g.getWidth();}
				if(unsafe_left < 0){unsafe_left = 0;}

				// 7.  
				for(int i = std::floor(unsafe_left); i < std::ceil(unsafe_right); i++){
					unsafe_spots.at(i) = 1;
				}
				

			}

```

#### Part 3. Determine Safe Spots (2): 

**Purpose:** Also mark the current explosion areas as unsafe.

**Steps:**
1. Obtain all the explosions.
2. Loop through all current explosions.
3. Find the interval of unsafe spots, as determined by the explosion size.
4. Clip the explosions left and right, if the window goes off the screen.
5. Update the unsafe spots, using this window.

```cpp
        // 1. 
		std::list<Explosion> explosion_list = g.getExplosions();

		// 2.  
		std::list<Explosion>::iterator explosion;
		for(explosion = explosion_list.begin(); explosion != explosion_list.end(); explosion++){
			
			// 3. 
			float unsafe_right = explosion->x + g.explosionSize;
			float unsafe_left = explosion->x - g.explosionSize;

			// 4.
			if(unsafe_right > g.getWidth()){unsafe_right = g.getWidth();}
			if(unsafe_left < 0){unsafe_left = 0;}

			// 5. 
			for(int i = std::floor(unsafe_left); i < std::ceil(unsafe_right); i++){
				unsafe_spots.at(i) = 1;
			}

		}
```

#### Part 4. **Pick a Safe Spot:** 

**Purpose:** Have the player either go to the left or right depending on the what unsafe areas are near the player.

**Steps:**
1. Get the list of unsafe spots and players.
2. For all players, find the current position.
3. Initialize small windows on the left and right of the current position. 
4. For both windows, set the window size by the explosion size. If the explosion size is too large and goes outside the bounds, then clip it.
5. For both windows, loop through those positions and take a cumulative sum of unsafe spots. Buffer this by 2 times to be extra safe.
6. If there are more unsafe spots to the left, go the right (and vice versa).

```cpp
int Controller::pickSafeSpot(const Game& g)
	{	
        // 1.
		std::vector<Player*> players_list = g.getPlayers();
		std::vector<bool> unsafe_spots = determineSafeSpots(g);

		for(int i = 0; i < players_list.size(); i++){
			
			// 2. 
			int player_pos = std::floor(players_list[i]->x);


			if(unsafe_spots.at(player_pos)){

				// 3.
				int unsafe_right = 0;
				int unsafe_left = 0;

				// 4. 
				float lim_right = g.explosionSize * 2;
				if(player_pos + g.explosionSize > g.getWidth()){

					lim_right = g.getWidth() - player_pos;
					unsafe_right = unsafe_right + g.explosionSize * 2;
				}

				float lim_left = g.explosionSize * 2;
				if(player_pos - g.explosionSize < 0){

					lim_left = player_pos - g.explosionSize;
					unsafe_left = unsafe_left + g.explosionSize * 2;

				}

                // 5. 				
				for(int i=1; i<lim_right; ++i){
					unsafe_right = unsafe_right + unsafe_spots.at(player_pos + i);
				} 
				
				for(int i=1; i<lim_left; ++i){
					unsafe_left = unsafe_left + unsafe_spots.at(player_pos - i);
				}

				// 6.
				if(unsafe_right >= unsafe_left){
					players_list[0] -> x = players_list[0] -> x - g.playerSpeed;
				}
				else{
					players_list[0] -> x = players_list[0] -> x + g.playerSpeed;
				}

			}

		}


		return 0;
	}
```

<hr>

## Q2: Challenges Faced


### *Q. What challenges did you face when writing an AI?.*

**Ans.** 
Challenges:
1. Effective Scoping
    - At a high level, first understanding the pertinent information I should process was a fun mental exercise. Essentially, what data structure should I decide to choose to encode safe spots and also how should the AI choose a safe spot?
    - For example, I could have preferred to make a 2D array that encoded position over a finite time, and then used a path-planning algorithm to maneuver to a safe zone.
    - However, given the time constaints I opted for a more simplistic data structure that was compliant with the instructions.
2. C++ Syntax
    - This was my first time dealing with a complex code structure in C++, so understanding its implementation and what capabilites it has.
    - Also, pointers and references was a new topic that took me a some reading to be comfortable with.
3. Hyperparameter Tuning
    - Because of my simplistic AI implementation, it took me a minute to understand which hyperparameters (window size, unsafe spot size, time window) were the right balance to maneuver at the medium level.

<hr>

## Q3: Challenges Faced


### *Q. How well does your AI work on a Hard scenario? If it doesn’t work, why? If it does, try harder scenarios and see when it does fail and explain why?.*

**Ans.** 
My AI did not work for harder levels. I believe this to be because of the simplicity in my implementation. My algorithm only constitutes a 1D array based on looking 10 seconds into the future. The works fine for the medium case, but for the harder cases when more projectiles exists, this fails.
<br>
I believe this to be because with multiple projectiles stacked on top of each other, you need to consider time as a factor. This would require a 2D array where you store both position, but also time. This would allow you to plan in two dimensions: both space and time. My AI simply shifts to the left or right based on relative positional safeness.


<hr>

## Q4: Assignment Thoughts


### *Q. What did you think of the assignment and did it meet its goals? Why or why not?.*

**Ans.** 
I really appreciated this assignment as it required a high level of syntacical awareness. I also appreciated that it was a goal driven assignment. It gave us a lot of the architecture in place - such that I didn't need to know about cmake but I could compile an entire video game. 
<br>
My only suggestions were that, preceeding the assignment, more time had been taken to go over the high-level topics in C++. Things like pointers, references, memory-allocation, STD, etc. Also, some basic debugging tools that can be used for C++. All my previous work had been in Visual Studio, so I never had to configure a debugging file. This made it very difficult to debug my file line-by-line without printing values.


