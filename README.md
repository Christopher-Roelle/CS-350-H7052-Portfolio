# CS-350-H7052-Portfolio

## Summarize the project and what problem it was solving.
The Morse Code Project handled being able to display Morse Code via 2 LEDs, and can be used to display any message via the use of stored messages.

The Smart Thermostat Project aimed to create a Smart Thermostat that could communicate with a server via Wireless Communication. The project currently uses the UART to display the data,
but has an empty function that can be expanded called UpdateServer() that is called every second which can be used to handle the needed data Tx.

## What did you do particularly well?
I think I excelled in the State-Machine department.

I enjoyed writing the State Machines and finding clever ways to manage the state that my code was in at any given time.
Like in the Morse Code example, using the current index within a string as the state control, and then having the before/end of the state available for transitioning to a new string if needed (check if an interrupt had fired during execution).

In the Smart Thermostat, I utilized an IDLE state for all interupt checks, where they could all be processed, which would move to the specified state for that action then back.
The interupt sequence checks were all handled inside the TimerCallback function, where individual counters based on 100ms period would tic up until 'x' * 100 ms had occured, then call the interupt check.
This provided a small data impact, since the counters were a very small number and just scaled up for the check.

## Where could you improve?
Further modularization through more C files and just import them.
This would allow my code to be further plug-and-play for other systems and architectures.

## What tools and/or resources are you adding to your support network?
I now understand how bitwise operations work.
I am now more concious of the RAM footprint that my code takes up.
Expanding on the above, I have learned how to better utilize the space the char datatype allocates and use it more intelligently to
minimize my data footprint. I.e. using char for a value greater than 255 and using a scalar to make it meet my requirements without
storing it long-term in RAM as a variable.

## What skills from this project will be particularly transferable to other projects and/or course work?
These projects have taught me skills such as bitwise operations which will provide efficiency gains in further projects down the road.
I will be able to find ways to work in smaller data types (such as char) to reduce memory requirements for all projects going forward, thus
allowing me to write smarter, more modular code that can work on lower-spec systems.

## How did you make this project maintainable, readable, and adaptable?
In the Morse Code LED project, I designed the system to map the word to a specific sequence on a letter basis.
You could expand this by mapping every alphabetical letter, and the program will be able to look up the letter and output the correct letter's morse code sequence via to Output GPIO.

For the Smart Thermostat, all functionality is cordoned into its own helper functions, including the Update Server code.
This could futher be modularized via the use of extra C files, but in the limitted timeframe, and for the sake of smaller data footprint, this is all handled via one Source File.
