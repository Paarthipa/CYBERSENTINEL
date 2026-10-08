#!/usr/bin/env python3

import argparse
import csv
import random
import time

from datetime import datetime, timedelta
from pathlib import Path


PATTERNS = [
    "NORMAL",
    "SCANNING",
    "BURST",
    "HIGH_BYTES",
    "FAILED_CONNECTION",
    "MIXED"
]


def generate_record(
    index,
    pattern,
    rng,
    start_time
):
    source_ip = (
        f"192.168.1.{rng.randint(1, 254)}"
    )

    destination_ip = (
        f"10.0.0.{rng.randint(1, 254)}"
    )

    source_port = rng.randint(
        1024,
        65535
    )

    destination_port = rng.choice(
        [22, 25, 53, 80, 123, 443]
    )

    protocol = (
        "UDP"
        if rng.random() < 0.2
        else "TCP"
    )

    if pattern == "NORMAL":

        packets = rng.randint(5, 30)

        byte_count = (
            packets *
            rng.randint(80, 200)
        )

        duration = rng.uniform(
            0.2,
            2.0
        )

        status = "SUCCESS"

    elif pattern == "SCANNING":

        packets = rng.randint(1, 10)

        byte_count = (
            packets *
            rng.randint(80, 200)
        )

        duration = rng.uniform(
            0.01,
            0.2
        )

        destination_port = rng.randint(
            1024,
            65535
        )

        status = "SUCCESS"

    elif pattern == "BURST":

        packets = rng.randint(
            100,
            500
        )

        byte_count = (
            packets *
            rng.randint(80, 160)
        )

        duration = rng.uniform(
            0.01,
            0.05
        )

        status = "SUCCESS"

    elif pattern == "HIGH_BYTES":

        packets = rng.randint(
            500,
            2000
        )

        byte_count = rng.randint(
            500_000,
            5_000_000
        )

        duration = rng.uniform(
            0.5,
            3.0
        )

        status = "SUCCESS"

    elif pattern == "FAILED_CONNECTION":

        packets = rng.randint(
            10,
            80
        )

        byte_count = (
            packets *
            rng.randint(80, 140)
        )

        duration = rng.uniform(
            0.1,
            1.0
        )

        status = (
            "FAILED"
            if rng.random() < 0.8
            else "SUCCESS"
        )

    else:

        pattern_choice = rng.choice(
            [
                "SCANNING",
                "BURST",
                "HIGH_BYTES",
                "FAILED_CONNECTION"
            ]
        )

        return generate_record(
            index,
            pattern_choice,
            rng,
            start_time
        )

    timestamp = (
        start_time +
        timedelta(
            milliseconds=index
        )
    )

    return [
        timestamp.isoformat(
            timespec="milliseconds"
        ),

        source_ip,
        destination_ip,

        source_port,
        destination_port,

        protocol,

        packets,
        byte_count,

        f"{duration:.4f}",

        status
    ]


def print_generation_summary(
    args,
    output_path,
    pattern_counts,
    generation_time,
    completed,
    records_processed
):
    minutes = int(
        generation_time // 60
    )

    seconds = (
        generation_time % 60
    )

    print()
    print(
        "============================================"
    )

    if completed:
        print(
            "       CYBERSENTINEL DATA GENERATOR"
        )
    else:
        print(
            "   CYBERSENTINEL GENERATION INTERRUPTED"
        )

    print(
        "============================================"
    )

    print(
        f"Records requested : "
        f"{args.records:,}"
    )

    print(
        f"Records processed : "
        f"{records_processed:,}"
    )

    for pattern in PATTERNS:

        print(
            f"{pattern:<18}: "
            f"{pattern_counts[pattern]:,}"
        )

    print(
        f"Output            : "
        f"{output_path}"
    )

    print()

    if completed:

        print(
            "Generation complete."
        )

        print(
            f"Generation time   : "
            f"{minutes} min {seconds:.2f} sec"
        )

        print(
            f"Generation time   : "
            f"{generation_time:.2f} seconds"
        )

        print(
            "Status            : COMPLETED"
        )

    else:

        print(
            "Generation stopped by user."
        )

        print(
            f"Elapsed time      : "
            f"{minutes} min {seconds:.2f} sec"
        )

        print(
            f"Elapsed time      : "
            f"{generation_time:.2f} seconds"
        )

        print(
            "Status            : INTERRUPTED"
        )


def main():

    # Start measuring the complete
    # dataset-generation process.
    generation_start = time.perf_counter()

    parser = argparse.ArgumentParser(
        description=
        "Generate synthetic CYBERSENTINEL network-flow data."
    )

    parser.add_argument(
        "--records",
        type=int,
        required=True
    )

    parser.add_argument(
        "--output",
        required=True
    )

    parser.add_argument(
        "--seed",
        type=int,
        default=42
    )

    args = parser.parse_args()

    if args.records <= 0:

        raise SystemExit(
            "--records must be greater than zero."
        )

    output_path = Path(
        args.output
    )

    output_path.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    rng = random.Random(
        args.seed
    )

    start_time = datetime(
        2026,
        10,
        5,
        21,
        0,
        0
    )

    pattern_counts = {
        pattern: 0
        for pattern in PATTERNS
    }

    records_processed = 0

    try:

        with output_path.open(
            "w",
            newline=""
        ) as file:

            writer = csv.writer(file)

            writer.writerow(
                [
                    "timestamp",
                    "source_ip",
                    "destination_ip",
                    "source_port",
                    "destination_port",
                    "protocol",
                    "packet_count",
                    "byte_count",
                    "duration",
                    "status"
                ]
            )

            for index in range(
                args.records
            ):

                fraction = (
                    index /
                    args.records
                )

                if fraction < 0.55:

                    pattern = "NORMAL"

                elif fraction < 0.67:

                    pattern = "SCANNING"

                elif fraction < 0.78:

                    pattern = "BURST"

                elif fraction < 0.86:

                    pattern = "HIGH_BYTES"

                elif fraction < 0.95:

                    pattern = (
                        "FAILED_CONNECTION"
                    )

                else:

                    pattern = "MIXED"

                pattern_counts[
                    pattern
                ] += 1

                record = generate_record(
                    index,
                    pattern,
                    rng,
                    start_time
                )

                writer.writerow(
                    record
                )

                records_processed = (
                    index + 1
                )

    except KeyboardInterrupt:

        generation_time = (
            time.perf_counter()
            - generation_start
        )

        print_generation_summary(
            args,
            output_path,
            pattern_counts,
            generation_time,
            False,
            records_processed
        )

        return

    generation_time = (
        time.perf_counter()
        - generation_start
    )

    print_generation_summary(
        args,
        output_path,
        pattern_counts,
        generation_time,
        True,
        records_processed
    )


if __name__ == "__main__":
    main()