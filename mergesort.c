#include <stdio.h>
#include <stdlib.h>

void
merge (int values[], int start, int mid, int end)
{
    int left_size  = mid - start + 1;
    int right_size = end - mid;
    int left[left_size];
    int right[right_size];

    // Copy data to temporary arrays left[] and right[]
    for (int i = 0; i < left_size; i++)
    {
        left[i] = values[start + i];
    }

    for (int i = 0; i < right_size; i++)
    {
        right[i] = values[mid + 1 + i];
    }

    // Merge the temporary arrays back into values[start...end]
    int i = 0, j = 0, k = start;

    while (i < left_size && j < right_size)
    {
        if (left[i] <= right[j])
        {
            values[k] = left[i];
            i++;
        }
        else
        {
            values[k] = right[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of left[], if any
    while (i < left_size)
    {
        values[k] = left[i];
        i++;
        k++;
    }

    // Copy the remaining elements of right[], if any
    while (j < right_size)
    {
        values[k] = right[j];
        j++;
        k++;
    }
}

void
merge_sort (int values[], int start, int end)
{
    if (start < end)
    {
        int mid = start + (end - start) / 2;

        merge_sort(values, start, mid);
        merge_sort(values, mid + 1, end);

        merge(values, start, mid, end);
    }
}

int
main ()
{
    int values[6] = { 6, 8, 2, 4, 1, 3 };
    int n         = 6;

    printf("Unsorted array: ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", values[i]);
    }
    printf("\n");

    merge_sort(values, 0, n - 1);

    printf("Sorted array:   ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", values[i]);
    }
    printf("\n");

    return 0;
}