# \# Swift-Ex: Advanced Logistics \& Courier Management System

# 

# \*\*Swift-Ex\*\* is a high-performance C++ logistics application designed to simulate a modern courier service. It handles the entire parcel lifecycle—from intake and priority sorting to dynamic routing and real-time delivery tracking—using custom-built Data Structures and Algorithms.



# \##  Key Features

# 

# \* \*\*Intelligent Parcel Sorting:\*\* Implements a \*\*Max-Heap\*\* to ensure high-priority and express parcels are dispatched first.

# \* \*\*Dynamic Routing Engine:\*\* A \*\*Graph-based\*\* system using DFS to find multiple routes. It detects road blockages or traffic overloads and reroutes parcels in real-time.

# \* \*\*Real-Time Transit Monitor:\*\* A live simulation of parcel movement with visual progress bars and ETA updates.

# \* \*\*Rider Assignment Logic:\*\* A management system that matches parcels to riders based on weight categories (Light vs. Heavy) and current workload.

# \* \*\*System Database:\*\* A custom \*\*Hash Table\*\* using quadratic probing for near-instant parcel lookups by ID.

# \* \*\*Undo Mechanism:\*\* A \*\*Stack-based\*\* action history allowing users to revert dispatches or accidental entries.




# \##  Data Structures Implemented

# 

# This project avoids STL containers to demonstrate a deep understanding of memory management and DSA:

# 

# | Data Structure | Implementation | Purpose |

# | :--- | :--- | :--- |

# | \*\*Max-Heap\*\* | `ParcelHeap` | Priority-based warehouse sorting. |

# | \*\*Hash Table\*\* | `ParcelHashTable` | $O(1)$ average time complexity for parcel tracking. |

# | \*\*Graph\*\* | `MapGraph` | Modeling the logistics network of cities and roads. |

# | \*\*Stack\*\* | `ActionStack` | Implementing the "Undo" functionality. |

# | \*\*Linked List\*\* | `ParcelLinkedList` | Managing parcels currently in transit. |

# | \*\*ArrayList\*\* | `ParcelArrayList` | Dynamic internal storage for various system nodes. |

# 

# 

# \##  Logistics Network

# 

# The system is pre-loaded with a comprehensive map of Pakistan, including:

# \* \*\*Cities:\*\* Lahore (Hub), Islamabad, Karachi, Peshawar, Quetta, and 20+ other locations.

# \* \*\*Zones:\*\* Cities are categorized into Zones (A, B, C, D) for regional logistics.

# \* \*\*Edge Weights:\*\* Roads have assigned distances (km) and "Max Load" capacities.

# 

# \### Dynamic Routing Logic

# The system calculates priority scores using the following logic:

# $$PriorityScore = (Priority \\times 1000) + Weight$$

# 

# If a road becomes \*\*Overloaded\*\* (too many active parcels) or \*\*Blocked\*\*, the `LogisticsEngine` triggers a re-calculation to find the next shortest available path.

# 

# 

# 

# \##  Technical Requirements

# 

# \* \*\*Language:\*\* C++11 or higher.

# \* \*\*Compiler:\*\* GCC / MinGW / Clang.

# \* \*\*Platform:\*\* Windows or Linux (Cross-platform terminal clearing supported).

# 

# 

# 

# \##  How to Run

# 

# 1\.  \*\*Compile:\*\*

# &nbsp;   ```bash

# &nbsp;   g++ main.cpp -o SwiftEx

# &nbsp;   ```

# 2\.  \*\*Execute:\*\*

# &nbsp;   ```bash

# &nbsp;   ./SwiftEx

# &nbsp;   ```

# 3\.  \*\*Basic Workflow:\*\*

# &nbsp;   \* \*\*Step 1:\*\* Use the \*\*Sorting Module\*\* to add a new pickup.

# &nbsp;   \* \*\*Step 2:\*\* Use the \*\*Courier Engine\*\* to assign a rider and dispatch.

# &nbsp;   \* \*\*Step 3:\*\* Open the \*\*Live Transit Monitor\*\* in the Tracking Module to watch the delivery progress.

# 

# 

# \##  Simulation Features

# \* \*\*Signal Loss:\*\* Parcels have a small random chance to go "Missing," requiring a status investigation.

# \* \*\*Delivery Attempts:\*\* If a "Recipient is Unavailable," the system automatically re-attempts delivery up to 3 times before returning the parcel to the sender.

