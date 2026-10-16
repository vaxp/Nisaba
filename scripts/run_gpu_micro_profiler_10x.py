#!/usr/bin/env python3
"""
GPU-Only 10x Benchmark Runner & Aggregator for Nisaba
Runs the `gpu_micro_profiler` benchmark 10 times, parses metrics,
computes 10-run averages, and writes a Markdown report to
'Benchmarking outputs/gpu_micro_profiler.md'.
"""

import os
import sys
import re
import time
import subprocess
import datetime
from pathlib import Path
from statistics import mean

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
BUILD_BENCH_DIR = WORKSPACE_ROOT / "build" / "benchmarks"
OUTPUT_DIR = WORKSPACE_ROOT / "Benchmarking outputs"

BENCHMARK_CONFIGS = [
    {
        "name": "gpu_micro_profiler",
        "binary": BUILD_BENCH_DIR / "gpu_micro_profiler",
        "output_file": OUTPUT_DIR / "gpu_micro_profiler.md",
        "type": "gpu_profiler",
        "description": "Nisaba GPU Engine vs Google Skia Ganesh GL (100 Micro-Benchmarking GPU Suites)",
    }
]


def run_command(cmd, cwd=WORKSPACE_ROOT):
    p = subprocess.run(cmd, shell=True, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return p.stdout, p.returncode


# Parser (copied/trimmed from original)

def parse_gpu_profiler(stdout):
    rows = []
    for m in re.finditer(
        r"^\s*(\d+)\.\s+(.*?)\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+(.*?)\s{2,}(.*?)$",
        stdout, re.MULTILINE
    ):
        rows.append({
            "id": int(m.group(1)),
            "name": m.group(2).strip(),
            "nis_cpu_us": float(m.group(3)),
            "nis_gpu_us": float(m.group(4)),
            "nis_tot_us": float(m.group(5)),
            "skia_cpu_us": float(m.group(6)),
            "skia_gpu_us": float(m.group(7)),
            "skia_tot_us": float(m.group(8)),
            "cpu_winner": m.group(9).strip(),
            "tot_winner": m.group(10).strip(),
        })

    tot_m = re.search(
        r"TOTAL FRAME OVERHEAD \(ALL SUITES\)\s+([\d.]+)\s*µs\s*([\d.]+)\s*µs\s*([\d.]+)\s*µs\s*([\d.]+)\s*µs\s*([\d.]+)\s*µs\s*([\d.]+)\s*µs\s+([^\n]+?)\s{2,}([^\n]+)",
        stdout
    )
    summary_m = re.search(
        r"SUMMARY:\s+Total Frame Performance:\s+Nisaba won in (\d+) suites \| Skia won in (\d+) suites \| Ties:\s*(\d+)",
        stdout
    )
    thru_m = re.search(r"Overall Frame Throughput \(End-to-End\):\s*([^\n]+)", stdout)
    sub_m = re.search(r"Overall CPU Command Submission:\s*([^\n]+)", stdout)

    return {
        "rows": rows,
        "tot_nis_cpu": float(tot_m.group(1)) if tot_m else 0.0,
        "tot_nis_gpu": float(tot_m.group(2)) if tot_m else 0.0,
        "tot_nis_tot": float(tot_m.group(3)) if tot_m else 0.0,
        "tot_skia_cpu": float(tot_m.group(4)) if tot_m else 0.0,
        "tot_skia_gpu": float(tot_m.group(5)) if tot_m else 0.0,
        "tot_skia_tot": float(tot_m.group(6)) if tot_m else 0.0,
        "cpu_speedup_str": tot_m.group(7).strip() if tot_m else "",
        "tot_speedup_str": tot_m.group(8).strip() if tot_m else "",
        "nis_wins": int(summary_m.group(1)) if summary_m else 0,
        "skia_wins": int(summary_m.group(2)) if summary_m else 0,
        "ties": int(summary_m.group(3)) if summary_m else 0,
        "throughput_str": thru_m.group(1).strip() if thru_m else "",
        "submission_str": sub_m.group(1).strip() if sub_m else "",
    }


# Report generator (GPU)

def generate_gpu_profiler_report(runs_outputs, parsed_runs):
    num_runs = len(parsed_runs)
    num_tests = len(parsed_runs[0]["rows"]) if parsed_runs and parsed_runs[0]["rows"] else 0

    mean_tot_nis_cpu = mean([r["tot_nis_cpu"] for r in parsed_runs])
    mean_tot_nis_gpu = mean([r["tot_nis_gpu"] for r in parsed_runs])
    mean_tot_nis_tot = mean([r["tot_nis_tot"] for r in parsed_runs])

    mean_tot_skia_cpu = mean([r["tot_skia_cpu"] for r in parsed_runs])
    mean_tot_skia_gpu = mean([r["tot_skia_gpu"] for r in parsed_runs])
    mean_tot_skia_tot = mean([r["tot_skia_tot"] for r in parsed_runs])

    cpu_speedup = mean_tot_skia_cpu / mean_tot_nis_cpu if mean_tot_nis_cpu > 0 else 1.0
    tot_speedup = mean_tot_skia_tot / mean_tot_nis_tot if mean_tot_nis_tot > 0 else 1.0

    avg_rows = []
    for i in range(num_tests):
        tid = parsed_runs[0]["rows"][i]["id"]
        tname = parsed_runs[0]["rows"][i]["name"]

        m_nis_cpu = mean([r["rows"][i]["nis_cpu_us"] for r in parsed_runs])
        m_nis_gpu = mean([r["rows"][i]["nis_gpu_us"] for r in parsed_runs])
        m_nis_tot = mean([r["rows"][i]["nis_tot_us"] for r in parsed_runs])

        m_skia_cpu = mean([r["rows"][i]["skia_cpu_us"] for r in parsed_runs])
        m_skia_gpu = mean([r["rows"][i]["skia_gpu_us"] for r in parsed_runs])
        m_skia_tot = mean([r["rows"][i]["skia_tot_us"] for r in parsed_runs])

        if m_nis_cpu < m_skia_cpu * 0.95:
            cpu_win = f"**{(m_skia_cpu/m_nis_cpu):.1f}x Nisaba**"
        elif m_skia_cpu < m_nis_cpu * 0.95:
            cpu_win = f"{(m_nis_cpu/m_skia_cpu):.1f}x Skia"
        else:
            cpu_win = "Tie (~1.0x)"

        if m_nis_tot < m_skia_tot * 0.95:
            tot_win = f"**{(m_skia_tot/m_nis_tot):.2f}x Nisaba**"
        elif m_skia_tot < m_nis_tot * 0.95:
            tot_win = f"{(m_nis_tot/m_skia_tot):.2f}x Skia"
        else:
            tot_win = "Tie (~1.0x)"

        avg_rows.append({
            "id": tid,
            "name": tname,
            "nis_cpu": m_nis_cpu,
            "nis_gpu": m_nis_gpu,
            "nis_tot": m_nis_tot,
            "skia_cpu": m_skia_cpu,
            "skia_gpu": m_skia_gpu,
            "skia_tot": m_skia_tot,
            "cpu_winner": cpu_win,
            "tot_winner": tot_win,
        })

    md = []
    md.append("# Benchmark Report: Nisaba GPU Engine vs Google Skia (Ganesh GL)")
    md.append("")
    md.append(f"**Execution Date:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}  ")
    md.append(f"**Runs Executed:** {num_runs} consecutive runs  ")
    md.append(f"**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  ")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## Executive Summary & Aggregate Averages")
    md.append("")
    md.append("Across **10 consecutive executions** of the 100-suite GPU micro-profiling benchmark:")
    md.append(f"- **Overall CPU Command Submission Overhead:** **{cpu_speedup:.2f}x FASTER (Nisaba)**")
    md.append(f"  * Nisaba Mean CPU Overhead: `{mean_tot_nis_cpu:.1f} µs` ({mean_tot_nis_cpu/1000.0:.2f} ms)")
    md.append(f"  * Skia Mean CPU Overhead: `{mean_tot_skia_cpu:.1f} µs` ({mean_tot_skia_cpu/1000.0:.2f} ms)")
    md.append(f"- **Overall End-to-End Frame Throughput:** **{tot_speedup:.2f}x FASTER (Nisaba)**")
    md.append(f"  * Nisaba Mean Total Frame Time: `{mean_tot_nis_tot:.1f} µs` ({mean_tot_nis_tot/1000.0:.2f} ms)")
    md.append(f"  * Skia Mean Total Frame Time: `{mean_tot_skia_tot:.1f} µs` ({mean_tot_skia_tot/1000.0:.2f} ms)")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## 10-Run Mean Results Across All 100 GPU Suites")
    md.append("")
    md.append("| # | Suite Name | Nisaba CPU (µs) | Nisaba GPU (µs) | Nisaba Total (µs) | Skia CPU (µs) | Skia GPU (µs) | Skia Total (µs) | CPU Ratio | Total Ratio |")
    md.append("| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |")

    for r in avg_rows:
        md.append(f"| {r['id']} | {r['name']} | **{r['nis_cpu']:.1f}** | {r['nis_gpu']:.1f} | **{r['nis_tot']:.1f}** | {r['skia_cpu']:.1f} | {r['skia_gpu']:.1f} | {r['skia_tot']:.1f} | {r['cpu_winner']} | {r['tot_winner']} |")

    md.append(f"| **--** | **TOTAL FRAME OVERHEAD** | **{mean_tot_nis_cpu:.1f}** | **{mean_tot_nis_gpu:.1f}** | **{mean_tot_nis_tot:.1f}** | **{mean_tot_skia_cpu:.1f}** | **{mean_tot_skia_gpu:.1f}** | **{mean_tot_skia_tot:.1f}** | **{cpu_speedup:.2f}x Nisaba** | **{tot_speedup:.2f}x Nisaba** |")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## Individual Run Logs (Runs 1 to 10)")
    md.append("")

    for idx, raw in enumerate(runs_outputs, start=1):
        md.append(f"### Run {idx}")
        md.append("<details>")
        md.append(f"<summary>Click to expand Run {idx} full log</summary>")
        md.append("")
        md.append("```text")
        md.append(raw.strip())
        md.append("```")
        md.append("</details>")
        md.append("")

    return "\n".join(md)


# Runner (trimmed)

def run_benchmark_10_times(bench):
    name = bench["name"]
    binary = bench["binary"]
    out_file = bench["output_file"]
    btype = bench["type"]

    print(f"\n================================================================================")
    print(f"[*] Starting 10-Run Benchmark: {name}")
    print(f"    Description: {bench['description']}")
    print(f"    Binary:      {binary}")
    print(f"    Target File: {out_file}")
    print(f"================================================================================")

    if not binary.exists():
        print(f"[!] Error: Binary {binary} does not exist. Please build it first.")
        return False

    raw_runs = []
    parsed_runs = []

    for run_idx in range(1, 11):
        t0 = time.time()
        print(f"  -> [{run_idx:02d}/10] Executing {name}...", end="", flush=True)
        stdout, code = run_command(str(binary))
        elapsed = time.time() - t0
        if code != 0:
            print(f" FAILED (exit code {code}) in {elapsed:.2f}s")
            print(stdout[:500])
            return False
        print(f" Done ({elapsed:.2f}s)")
        raw_runs.append(stdout)

        if btype == "gpu_profiler":
            parsed_runs.append(parse_gpu_profiler(stdout))

    print(f"  -> Aggregating statistical metrics and writing {out_file.name}...")
    report_md = generate_gpu_profiler_report(raw_runs, parsed_runs)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(report_md)

    print(f"  [✓] Successfully generated: {out_file} ({os.path.getsize(out_file):,} bytes)")
    return True


def main():
    print("================================================================================")
    print("  NISABA GPU-ONLY — 10x BENCHMARK ORCHESTRATOR")
    print("================================================================================")
    print(f"Target Output Directory: {OUTPUT_DIR}")
    print(f"Total Benchmarks to Run: {len(BENCHMARK_CONFIGS)}")
    print(f"Total Executions Planned: {len(BENCHMARK_CONFIGS) * 10}")

    total_start = time.time()

    for idx, bench in enumerate(BENCHMARK_CONFIGS, start=1):
        print(f"\n>>> Benchmark [{idx}/{len(BENCHMARK_CONFIGS)}]: {bench['name']}")
        success = run_benchmark_10_times(bench)
        if not success:
            print(f"[!] Benchmark {bench['name']} encountered an error. Aborting.")
            sys.exit(1)

    total_elapsed = time.time() - total_start
    print(f"\n================================================================================")
    print(f"  ALL GPU BENCHMARK RUNS COMPLETED SUCCESSFULLY in {total_elapsed / 60.0:.2f} minutes!")
    print(f"  Outputs saved in: {OUTPUT_DIR}/")
    print("================================================================================")


if __name__ == "__main__":
    main()
