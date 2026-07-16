class Heap:

    def __init__(self, capacity):

        self.heap = [None] * capacity
        self.capacity = capacity
        self.size = 0

    def push(self, item):
        
        if self.size >= self.capacity:
            print('Heap ran out of storage')
            return

        # Insert at the end and sift up
        self.heap[self.size] = item
        self.size += 1
        self._sift_up(self.size - 1)

    def pop(self):
        """Removes and returns the minimum element from the heap."""
        if self.size == 0:
            raise IndexError("pop from an empty heap")
        
        min_val = self.heap[0]
        
        # Move the last element to the root and sift down
        self.heap[0] = self.heap[self.size - 1]
        self.size -= 1
        
        if self.size > 0:
            self._sift_down(0)
            
        return min_val

    def peek(self):
        """Returns the minimum element without removing it."""
        if self.size == 0:
            raise IndexError("peek from an empty heap")
        return self.heap[0]

    def isempty(self):
        return self.size == 0

    def _sift_up(self, idx):
        """Maintains min-heap property by moving an element upwards."""
        while idx > 0:
            parent = (idx - 1) // 2
            if not (self.heap[idx] < self.heap[parent]):
                break
            # Vectorized/fast swap in NumPy
            self.heap[idx], self.heap[parent] = self.heap[parent], self.heap[idx]
            idx = parent

    def _sift_down(self, idx):
        """Maintains min-heap property by moving an element downwards."""
        while True:
            left = 2 * idx + 1
            right = 2 * idx + 2
            smallest = idx

            if left < self.size and self.heap[left] < self.heap[smallest]:
                smallest = left
            if right < self.size and self.heap[right] < self.heap[smallest]:
                smallest = right

            if smallest == idx:
                break

            self.heap[idx], self.heap[smallest] = self.heap[smallest], self.heap[idx]
            idx = smallest

    def __len__(self):
        return self.size
