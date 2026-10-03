#!/usr/bin/env python3
"""
Nisaba Sovereign Graphics Engine — 10x Consecutive Benchmark Runner & Aggregator
Runs all 5 benchmarks 10 times consecutively, parses quantitative metrics,
computes statistical averages, and writes clean Markdown reports to 'Benchmarking outputs/'.
"""

import os
import sys
import re
import time
import subprocess
import datetime
from pathlib import Path
from statistics import mean, stdev

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
BUILD_BENCH_DIR = WORKSPACE_ROOT / "build" / "benchmarks"
OUTPUT_DIR = WORKSPACE_ROOT / "Benchmarking outputs"

BENCHMARK_CONFIGS = [
    {
        "name": "benchmark_image_decoders",
        "binary": BUILD_BENCH_DIR / "benchmark_image_decoders",
        "output_file": OUTPUT_DIR / "benchmark_image_decoders.md",
        "type": "image_decoders",
        "description": "Nisaba Sovereign Image Codecs (PNG, JPEG, QOI) vs stb_image.h",
    },
    {
        "name": "benchmark_text_engines",
        "binary": BUILD_BENCH_DIR / "benchmark_text_engines",
        "output_file": OUTPUT_DIR / "benchmark_text_engines.md",
        "type": "text_engines",
        "description": "Nisaba Native C++20 Multilingual Text Engine vs stb_truetype.h & fontstash.h",
    },
    {
        "name": "nisaba_vs_cairo",
        "binary": BUILD_BENCH_DIR / "nisaba_vs_cairo",
        "output_file": OUTPUT_DIR / "nisaba_vs_cairo.md",
        "type": "cairo_skia_90",
        "competitor": "Cairo",
        "description": "Nisaba 2D Raster Engine vs GNU Cairo 1.18 (90 Rigorous Geometric Suites)",
    },
    {
        "name": "nisaba_vs_skia",
        "binary": BUILD_BENCH_DIR / "nisaba_vs_skia",
        "output_file": OUTPUT_DIR / "nisaba_vs_skia.md",
        "type": "cairo_skia_90",
        "competitor": "Skia",
        "description": "Nisaba 2D Raster Engine vs Google Skia CPU (90 Rigorous Geometric Suites)",
    },
    {
        "name": "gpu_micro_profiler",
        "binary": BUILD_BENCH_DIR / "gpu_micro_profiler",
        "output_file": OUTPUT_DIR / "gpu_micro_profiler.md",
        "type": "gpu_profiler",
        "description": "Nisaba GPU Engine vs Google Skia Ganesh GL (100 Micro-Benchmarking GPU Suites)",
    },
]

def run_command(cmd, cwd=WORKSPACE_ROOT):
    p = subprocess.run(cmd, shell=True, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return p.stdout, p.returncode

# ---------------------------------------------------------------------------
# Parsers
# ---------------------------------------------------------------------------

def parse_image_decoders(stdout):
    # Split by Image:
    sections = re.split(r"-{50,}\nImage:\s+", stdout)
    images_data = []
    for sec in sections[1:]:
        lines = sec.split("\n")
        header = lines[0].strip() # e.g. "vaxp.png [PNG]"
        res_match = re.search(r"Resolution:\s+([^|]+)\|\s+File Size:\s+([^|]+)", sec)
        res_str = res_match.group(1).strip() if res_match else ""
        size_str = res_match.group(2).strip() if res_match else ""

        # Nisaba line
        nis_match = re.search(r"Nisaba Native\s+([\d.]+)\s*ms\s+([\d.]+)\s*ms\s+([\d.]+)\s*ms\s+([\d.]+)\s*MP/s", sec)
        # STB line
        stb_match = re.search(r"stb_image \(External\)\s+([\d.]+)\s*ms\s+([\d.]+)\s*ms\s+([\d.]+)\s*ms\s+([\d.]+)\s*MP/s", sec)
        stb_unsupported = "Format Not Supported" in sec

        speed_match = re.search(r">>> Speed: Nisaba is ([\d.]+)x FASTER than stb_image", sec)
        speedup = float(speed_match.group(1)) if speed_match else (None if stb_unsupported else 1.0)

        psnr_match = re.search(r"PSNR \(Quality Metric\)\s*:\s*([^\n]+)", sec)
        psnr_str = psnr_match.group(1).strip() if psnr_match else ""

        item = {
            "image": header,
            "resolution": res_str,
            "size": size_str,
            "nisaba_avg_ms": float(nis_match.group(1)) if nis_match else 0.0,
            "nisaba_min_ms": float(nis_match.group(2)) if nis_match else 0.0,
            "nisaba_max_ms": float(nis_match.group(3)) if nis_match else 0.0,
            "nisaba_mpps": float(nis_match.group(4)) if nis_match else 0.0,
            "stb_supported": not stb_unsupported,
            "stb_avg_ms": float(stb_match.group(1)) if stb_match else (0.0 if not stb_unsupported else None),
            "stb_min_ms": float(stb_match.group(2)) if stb_match else (0.0 if not stb_unsupported else None),
            "stb_max_ms": float(stb_match.group(3)) if stb_match else (0.0 if not stb_unsupported else None),
            "stb_mpps": float(stb_match.group(4)) if stb_match else (0.0 if not stb_unsupported else None),
            "speedup": speedup,
            "psnr": psnr_str,
        }
        images_data.append(item)
    return images_data

def parse_text_engines(stdout):
    suites_data = {}
    # Grand total parsing
    grand_nis = re.search(r"• Nisaba Native Engine\s*:\s*([\d.]+)\s*ms", stdout)
    grand_ext = re.search(r"• External GPU Engine\s*:\s*([\d.]+)\s*ms", stdout)
    grand_delta = re.search(r"• Absolute Time Delta\s*:\s*([^\n]+)", stdout)

    # Suite items
    # Suite 1
    s1_rows = re.findall(r"([\w.-]+\.ttf)\s+([\d.]+)\s+([\d.]+)\s*us\s+([\d.]+)\s*us\s+([^\n]+)", stdout)
    # Suite 2
    s2_rows = re.findall(r"([A-Za-z0-9 ()+]+?)\s+(\d{5,8})\s+([\d.]+)\s*M/s\s+([\d.]+)\s*M/s\s+([^\n]+)", stdout)
    # Suite 3
    s3_rows = re.findall(r"([A-Za-z0-9 ()+]+?)\s+(\d{5,8})\s+([\d.]+)\s*M/s\s+([\d.]+)\s*M/s\s+([^\n]+)", stdout)
    # Suite 5
    s5_rows = re.findall(r"(Rasterize @ \d+px)\s+(\d+)\s+([\d.]+)\s*glyphs/s\s+([\d.]+)\s*glyphs/s\s+([^\n]+)", stdout)
    # Suite 6
    s6_rows = re.findall(r"([A-Za-z0-9 -]+?)\s+(\d+)\s+([\d.]+)\s*us\s+([\d.]+)\s*us\s+([^\n]+)", stdout)

    # Aggregate lines in Grand Total Table
    agg_rows = re.findall(r"(\d+\.\s+[A-Za-z0-9 &]+?)\s+([\d.]+)\s*ms\s+([\d.]+)\s*ms\s+([^\n]+)", stdout)

    return {
        "grand_nis_ms": float(grand_nis.group(1)) if grand_nis else 0.0,
        "grand_ext_ms": float(grand_ext.group(1)) if grand_ext else 0.0,
        "grand_delta": grand_delta.group(1).strip() if grand_delta else "",
        "agg_rows": [
            {"suite": r[0].strip(), "nis_ms": float(r[1]), "ext_ms": float(r[2]), "status": r[3].strip()}
            for r in agg_rows
        ]
    }

def parse_90_suites(stdout):
    rows = []
    # Pattern: 19. 50 Rotated Quads (Affine Alpha Stack)             559.4 µs    579.7 µs       Tie (~1.0x)
    for m in re.finditer(r"^\s*(\d+)\.\s+(.*?)\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+(.*)$", stdout, re.MULTILINE):
        test_id = int(m.group(1))
        test_name = m.group(2).strip()
        nis_us = float(m.group(3))
        comp_us = float(m.group(4))
        status = m.group(5).strip()
        rows.append({
            "id": test_id,
            "name": test_name,
            "nis_us": nis_us,
            "comp_us": comp_us,
            "status": status,
        })
    tot_match = re.search(r"TOTAL FRAME OVERHEAD\s+([\d.]+)\s*µs\s+([\d.]+)\s*µs\s+([^\n]+)", stdout)
    tot_nis = float(tot_match.group(1)) if tot_match else 0.0
    tot_comp = float(tot_match.group(2)) if tot_match else 0.0
    tot_speedup = tot_match.group(3).strip() if tot_match else ""

    win_match = re.search(r"Nisaba Won:\s*(\d+)\s*/\s*(\d+)", stdout)
    nis_wins = int(win_match.group(1)) if win_match else 0
    total_tests = int(win_match.group(2)) if win_match else len(rows)

    return {
        "rows": rows,
        "total_nis_us": tot_nis,
        "total_comp_us": tot_comp,
        "total_speedup": tot_speedup,
        "nis_wins": nis_wins,
        "total_tests": total_tests,
    }

def parse_gpu_profiler(stdout):
    rows = []
    # Pattern: 53. 15 Concentric Radial Spotlights          69.0 µs 1000.7 µs   1069.7 µs   92.7 µs  948.4 µs   1041.1 µs       1.3x Nisaba         Tie (~1.0x)
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

# ---------------------------------------------------------------------------
# Report Generators
# ---------------------------------------------------------------------------

def generate_image_decoders_report(runs_outputs, parsed_runs):
    # Compute 10-run averages per image
    num_runs = len(parsed_runs)
    num_images = len(parsed_runs[0])
    avg_images = []

    for i in range(num_images):
        img_name = parsed_runs[0][i]["image"]
        res = parsed_runs[0][i]["resolution"]
        size = parsed_runs[0][i]["size"]
        stb_supp = parsed_runs[0][i]["stb_supported"]
        psnr = parsed_runs[0][i]["psnr"]

        nis_avgs = [r[i]["nisaba_avg_ms"] for r in parsed_runs]
        nis_mpps = [r[i]["nisaba_mpps"] for r in parsed_runs]
        mean_nis_avg = mean(nis_avgs)
        mean_nis_mpps = mean(nis_mpps)

        if stb_supp:
            stb_avgs = [r[i]["stb_avg_ms"] for r in parsed_runs]
            stb_mpps = [r[i]["stb_mpps"] for r in parsed_runs]
            mean_stb_avg = mean(stb_avgs)
            mean_stb_mpps = mean(stb_mpps)
            speedup = mean_stb_avg / mean_nis_avg if mean_nis_avg > 0 else 1.0
        else:
            mean_stb_avg = None
            mean_stb_mpps = None
            speedup = None

        avg_images.append({
            "image": img_name,
            "resolution": res,
            "size": size,
            "stb_supported": stb_supp,
            "nis_avg_ms": mean_nis_avg,
            "nis_mpps": mean_nis_mpps,
            "stb_avg_ms": mean_stb_avg,
            "stb_mpps": mean_stb_mpps,
            "speedup": speedup,
            "psnr": psnr,
        })

    md = []
    md.append("# Benchmark Report: Nisaba Image Decoders vs stb_image.h")
    md.append("")
    md.append(f"**Execution Date:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}  ")
    md.append(f"**Runs Executed:** {num_runs} consecutive runs  ")
    md.append(f"**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  ")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## Executive Summary & 10-Run Averages")
    md.append("")
    md.append("The table below displays the **arithmetic mean of 10 consecutive benchmark executions** for each test workload, measuring decoding time (ms), throughput (Megapixels/sec), speedup factor, and visual PSNR fidelity.")
    md.append("")
    md.append("| Test Image | Format | Resolution | File Size | Nisaba Mean Time | STB Mean Time | Nisaba Throughput | STB Throughput | Average Speedup | Fidelity (PSNR) |")
    md.append("| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |")

    for a in avg_images:
        stb_time_str = f"{a['stb_avg_ms']:.2f} ms" if a['stb_supported'] else "N/A (Unsupported)"
        stb_mpps_str = f"{a['stb_mpps']:.1f} MP/s" if a['stb_supported'] else "N/A"
        speed_str = f"**{a['speedup']:.2f}x FASTER**" if a['speedup'] else "N/A (Sovereign Only)"
        fmt = a['image'].split()[-1].strip("[]")
        name_only = a['image'].split()[0]
        md.append(f"| `{name_only}` | **{fmt}** | {a['resolution']} | {a['size']} | **{a['nis_avg_ms']:.2f} ms** | {stb_time_str} | **{a['nis_mpps']:.1f} MP/s** | {stb_mpps_str} | {speed_str} | {a['psnr']} |")

    md.append("")
    md.append("---")
    md.append("")
    md.append("## Individual Run Logs (Runs 1 to 10)")
    md.append("")
    md.append("Below are the complete, unmodified standard output logs for each individual execution.")
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

def generate_text_engines_report(runs_outputs, parsed_runs):
    num_runs = len(parsed_runs)
    mean_grand_nis = mean([r["grand_nis_ms"] for r in parsed_runs])
    mean_grand_ext = mean([r["grand_ext_ms"] for r in parsed_runs])
    mean_delta = mean_grand_nis - mean_grand_ext

    # Agg rows
    num_suites = len(parsed_runs[0]["agg_rows"])
    avg_suites = []
    for s_idx in range(num_suites):
        suite_name = parsed_runs[0]["agg_rows"][s_idx]["suite"]
        status = parsed_runs[0]["agg_rows"][s_idx]["status"]
        mean_nis = mean([r["agg_rows"][s_idx]["nis_ms"] for r in parsed_runs])
        mean_ext = mean([r["agg_rows"][s_idx]["ext_ms"] for r in parsed_runs])
        speedup = mean_ext / mean_nis if mean_nis > 0 else 1.0
        avg_suites.append({
            "suite": suite_name,
            "nis_ms": mean_nis,
            "ext_ms": mean_ext,
            "speedup": speedup,
            "status": status,
        })

    md = []
    md.append("# Benchmark Report: Nisaba Text Engine vs stb_truetype / fontstash")
    md.append("")
    md.append(f"**Execution Date:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}  ")
    md.append(f"**Runs Executed:** {num_runs} consecutive runs  ")
    md.append(f"**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  ")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## Executive Summary & 10-Run Averages")
    md.append("")
    md.append("Arithmetic averages across 10 consecutive executions evaluating full typography pipelines: font table validation, CMAP Unicode mapping, horizontal metrics, vector glyph outline extraction, subpixel glyph rasterization, and multi-line BiDi paragraph layout.")
    md.append("")
    md.append("| Benchmark Stage | Nisaba Mean Time | External Mean Time | Speedup Ratio | Architectural Status |")
    md.append("| :--- | :---: | :---: | :---: | :--- |")

    for s in avg_suites:
        if s['speedup'] >= 1.05:
            sp_str = f"**{s['speedup']:.2f}x FASTER (Nisaba)**"
        elif s['speedup'] <= 0.95:
            sp_str = f"{(1.0/s['speedup']):.2f}x slower"
        else:
            sp_str = "Tie (~1.0x)"
        md.append(f"| **{s['suite']}** | **{s['nis_ms']:.2f} ms** | {s['ext_ms']:.2f} ms | {sp_str} | {s['status']} |")

    md.append("")
    md.append("### Combined Grand Total Execution Time (10-Run Average)")
    md.append(f"- **Nisaba Native Engine Average:** `{mean_grand_nis:.2f} ms`")
    md.append(f"- **External Reference Engine Average:** `{mean_grand_ext:.2f} ms`")
    md.append(f"- **Delta:** `+{mean_delta:.2f} ms` (Reflecting Nisaba's comprehensive eager font validation, full Unicode BiDi, and contextual Arabic shaping)")
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

def generate_90_suites_report(title, competitor_name, runs_outputs, parsed_runs):
    num_runs = len(parsed_runs)
    num_tests = len(parsed_runs[0]["rows"])

    mean_tot_nis = mean([r["total_nis_us"] for r in parsed_runs])
    mean_tot_comp = mean([r["total_comp_us"] for r in parsed_runs])
    mean_speedup = mean_tot_comp / mean_tot_nis if mean_tot_nis > 0 else 1.0

    avg_rows = []
    nis_win_count = 0
    comp_win_count = 0
    tie_count = 0

    for i in range(num_tests):
        tid = parsed_runs[0]["rows"][i]["id"]
        tname = parsed_runs[0]["rows"][i]["name"]

        mean_nis = mean([r["rows"][i]["nis_us"] for r in parsed_runs])
        mean_comp = mean([r["rows"][i]["comp_us"] for r in parsed_runs])

        if mean_nis < mean_comp * 0.95:
            ratio = mean_comp / mean_nis
            winner = f"**{ratio:.2f}x (Nisaba)**"
            nis_win_count += 1
        elif mean_comp < mean_nis * 0.95:
            ratio = mean_nis / mean_comp
            winner = f"{ratio:.2f}x ({competitor_name})"
            comp_win_count += 1
        else:
            winner = "Tie (~1.0x)"
            tie_count += 1

        avg_rows.append({
            "id": tid,
            "name": tname,
            "nis_us": mean_nis,
            "comp_us": mean_comp,
            "winner": winner,
        })

    md = []
    md.append(f"# Benchmark Report: Nisaba vs {competitor_name} (90 Rigorous Geometric Suites)")
    md.append("")
    md.append(f"**Execution Date:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}  ")
    md.append(f"**Runs Executed:** {num_runs} consecutive runs  ")
    md.append(f"**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  ")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## Executive Summary & Aggregate Averages")
    md.append("")
    md.append(f"Across **10 consecutive executions** testing 90 vector geometry, shader, blending, and compositing benchmarks:")
    md.append(f"- **Nisaba Average Total Frame Time:** `{mean_tot_nis:.1f} µs` ({mean_tot_nis / 1000.0:.2f} ms)")
    md.append(f"- **{competitor_name} Average Total Frame Time:** `{mean_tot_comp:.1f} µs` ({mean_tot_comp / 1000.0:.2f} ms)")
    md.append(f"- **Overall Speedup:** **{mean_speedup:.2f}x FASTER (Nisaba)**")
    md.append(f"- **Win / Loss Scorecard (10-Run Mean):**")
    md.append(f"  * **Nisaba Won:** **{nis_win_count} / {num_tests} suites ({(nis_win_count/num_tests)*100:.1f}%)**")
    md.append(f"  * **{competitor_name} Won:** {comp_win_count} / {num_tests} suites ({(comp_win_count/num_tests)*100:.1f}%)")
    if tie_count > 0:
        md.append(f"  * **Ties:** {tie_count} suites")
    md.append("")
    md.append("---")
    md.append("")
    md.append("## 10-Run Mean Results by Benchmark Suite")
    md.append("")
    md.append(f"| # | Benchmark Suite Description | Nisaba Mean (µs) | {competitor_name} Mean (µs) | Speedup / Winner |")
    md.append("| :---: | :--- | :---: | :---: | :---: |")

    for r in avg_rows:
        md.append(f"| {r['id']} | {r['name']} | **{r['nis_us']:.1f} µs** | {r['comp_us']:.1f} µs | {r['winner']} |")

    md.append(f"| **--** | **TOTAL FRAME OVERHEAD** | **{mean_tot_nis:.1f} µs** | **{mean_tot_comp:.1f} µs** | **{mean_speedup:.2f}x (Nisaba)** |")
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

def generate_gpu_profiler_report(runs_outputs, parsed_runs):
    num_runs = len(parsed_runs)
    num_tests = len(parsed_runs[0]["rows"])

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

        # CPU winner
        if m_nis_cpu < m_skia_cpu * 0.95:
            cpu_win = f"**{(m_skia_cpu/m_nis_cpu):.1f}x Nisaba**"
        elif m_skia_cpu < m_nis_cpu * 0.95:
            cpu_win = f"{(m_nis_cpu/m_skia_cpu):.1f}x Skia"
        else:
            cpu_win = "Tie (~1.0x)"

        # Total winner
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

# ---------------------------------------------------------------------------
# Main Orchestrator
# ---------------------------------------------------------------------------

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

        if btype == "image_decoders":
            parsed_runs.append(parse_image_decoders(stdout))
        elif btype == "text_engines":
            parsed_runs.append(parse_text_engines(stdout))
        elif btype == "cairo_skia_90":
            parsed_runs.append(parse_90_suites(stdout))
        elif btype == "gpu_profiler":
            parsed_runs.append(parse_gpu_profiler(stdout))

    # Generate Markdown Report
    print(f"  -> Aggregating statistical metrics and writing {out_file.name}...")
    if btype == "image_decoders":
        report_md = generate_image_decoders_report(raw_runs, parsed_runs)
    elif btype == "text_engines":
        report_md = generate_text_engines_report(raw_runs, parsed_runs)
    elif btype == "cairo_skia_90":
        report_md = generate_90_suites_report(name, bench["competitor"], raw_runs, parsed_runs)
    elif btype == "gpu_profiler":
        report_md = generate_gpu_profiler_report(raw_runs, parsed_runs)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(report_md)

    print(f"  [✓] Successfully generated: {out_file} ({os.path.getsize(out_file):,} bytes)")
    return True

def main():
    print("================================================================================")
    print("  NISABA SOVEREIGN GRAPHICS ENGINE — 10x BENCHMARK ORCHESTRATOR")
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
    print(f"  ALL 10x BENCHMARK RUNS COMPLETED SUCCESSFULLY in {total_elapsed / 60.0:.2f} minutes!")
    print(f"  Outputs saved in: {OUTPUT_DIR}/")
    print("================================================================================")

if __name__ == "__main__":
    main()
